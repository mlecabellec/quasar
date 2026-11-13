#!/usr/bin/env python3
"""
Wrapper for mkdocs-kit CLI that rewrites internal cross-page links in the combined
PDF HTML into internal PDF fragment anchors (#pdf-page-...), so WeasyPrint emits
internal PDF GoTo link annotations instead of broken file/URI links.
"""

import posixpath
import re
import sys
import urllib.parse


_SVG_BLOCK_RE = re.compile(r"<svg\b[^>]*>.*?</svg>", re.DOTALL | re.IGNORECASE)
_SECTION_OPEN_RE = re.compile(
    r'(<section\b[^>]*\bclass=["\']pdf-page["\'][^>]*\bdata-source=["\']([^"\']+)["\'][^>]*>)'
)
_ID_ATTR_RE = re.compile(r'\bid=(["\'])([^"\']+)\1')
_HREF_ATTR_RE = re.compile(r'(\bhref=)(["\'])([^"\']*)\2')


def _slugify_page_id(rel_html: str) -> str:
    slug = re.sub(r"[^A-Za-z0-9_-]+", "_", rel_html).strip("_")
    return f"pdf-page-{slug or 'root'}"


def rewrite_combined_pdf_html(master_html: str) -> str:
    # Protect inline SVG blocks so internal SVG <defs> / url(#id) remain untouched.
    svg_blocks = []

    def _stash_svg(match: re.Match) -> str:
        idx = len(svg_blocks)
        svg_blocks.append(match.group(0))
        return f"<!--__QUASAR_SVG_{idx}__-->"

    protected_html = _SVG_BLOCK_RE.sub(_stash_svg, master_html)

    matches = list(_SECTION_OPEN_RE.finditer(protected_html))
    if not matches:
        for idx, block in enumerate(svg_blocks):
            protected_html = protected_html.replace(f"<!--__QUASAR_SVG_{idx}__-->", block)
        return protected_html

    # Slice master HTML into header, per-page sections, and footer.
    prefix = protected_html[: matches[0].start()]
    sections = []
    for i, m in enumerate(matches):
        seg_start = m.start()
        seg_end = matches[i + 1].start() if i + 1 < len(matches) else len(protected_html)
        rel_html = posixpath.normpath(m.group(2))
        open_tag = m.group(1)
        body = protected_html[m.end() : seg_end]
        sections.append((rel_html, open_tag, body))

    page_ids = {}
    id_map = {}

    # Pass 1: Assign deterministic page IDs and per-page namespaced element IDs.
    for rel_html, _open_tag, body in sections:
        page_id = _slugify_page_id(rel_html)
        page_ids[rel_html] = page_id
        if rel_html.endswith(".md"):
            page_ids[rel_html[:-3] + ".html"] = page_id
            if posixpath.basename(rel_html) in ("README.md", "index.md"):
                dir_name = posixpath.dirname(rel_html)
                page_ids[posixpath.join(dir_name, "index.html") if dir_name else "index.html"] = page_id
        for id_match in _ID_ATTR_RE.finditer(body):
            old_id = id_match.group(2)
            id_map[(page_id, old_id)] = f"{page_id}--{old_id}"

    # Pass 2: Namespace IDs and rewrite internal hrefs to #pdf-page-... / #pdf-page-...--frag.
    rebuilt_sections = []
    for rel_html, open_tag, body in sections:
        page_id = page_ids[rel_html]
        if ' id="' not in open_tag and " id='" not in open_tag:
            open_tag = open_tag[:-1] + f' id="{page_id}">'

        def _rewrite_id(m: re.Match) -> str:
            quote = m.group(1)
            old_id = m.group(2)
            new_id = id_map.get((page_id, old_id), old_id)
            return f"id={quote}{new_id}{quote}"

        body_ids = _ID_ATTR_RE.sub(_rewrite_id, body)

        def _rewrite_href(m: re.Match) -> str:
            import html as _html
            attr_eq, quote, href = m.group(1), m.group(2), m.group(3)
            unescaped_href = _html.unescape(href)
            if "mailto:" in unescaped_href and not unescaped_href.startswith(("http://", "https://")):
                mailto_idx = unescaped_href.index("mailto:")
                return f"{attr_eq}{quote}{unescaped_href[mailto_idx:]}{quote}"
            if not href or unescaped_href.startswith(("http://", "https://", "mailto:", "javascript:", "tel:", "data:")):
                return m.group(0)

            if href.startswith("#"):
                frag = urllib.parse.unquote(href[1:])
                mapped = id_map.get((page_id, frag))
                if mapped:
                    return f"{attr_eq}{quote}#{mapped}{quote}"
                return m.group(0)

            path_part, frag = urllib.parse.urldefrag(href)
            if not path_part:
                return m.group(0)

            dec_path = posixpath.normpath(urllib.parse.unquote(path_part))
            candidates = [dec_path]
            if dec_path.startswith("./"):
                candidates.append(dec_path[2:])
            if dec_path.endswith(".md"):
                candidates.append(dec_path[:-3] + ".html")
            if not dec_path.endswith(".html"):
                candidates.append(posixpath.join(dec_path, "index.html"))

            target_page_id = None
            for cand in candidates:
                if cand in page_ids:
                    target_page_id = page_ids[cand]
                    break

            if target_page_id is None:
                # Fallback: retry joining relative to current page directory in case adjust_paths skipped it.
                page_dir = posixpath.dirname(rel_html)
                joined = posixpath.normpath(posixpath.join(page_dir, dec_path))
                fallback_candidates = [joined]
                if joined.endswith(".md"):
                    fallback_candidates.append(joined[:-3] + ".html")
                elif not joined.endswith(".html"):
                    fallback_candidates.append(joined + ".html")
                    fallback_candidates.append(posixpath.join(joined, "index.html"))
                for cand in fallback_candidates:
                    if cand in page_ids:
                        target_page_id = page_ids[cand]
                        break

            if target_page_id is not None:
                frag_dec = urllib.parse.unquote(frag) if frag else ""
                if frag_dec and (target_page_id, frag_dec) in id_map:
                    return f"{attr_eq}{quote}#{id_map[(target_page_id, frag_dec)]}{quote}"
                return f"{attr_eq}{quote}#{target_page_id}{quote}"

            return m.group(0)

        body_links = _HREF_ATTR_RE.sub(_rewrite_href, body_ids)
        rebuilt_sections.append(open_tag + body_links)

    result = prefix + "".join(rebuilt_sections)
    for idx, block in enumerate(svg_blocks):
        result = result.replace(f"<!--__QUASAR_SVG_{idx}__-->", block)
    return result


def main() -> int:
    import os
    import shutil
    import yaml
    import weasyprint
    import mkdocs_kit.pdf as mk_pdf
    import mkdocs_kit.cli as mk_cli

    orig_html_cls = weasyprint.HTML
    orig_generate_pdf = mk_pdf.generate_pdf

    def _PatchedHTML(*args, **kwargs):
        if "string" in kwargs and isinstance(kwargs["string"], str):
            kwargs["string"] = rewrite_combined_pdf_html(kwargs["string"])
        elif args and isinstance(args[0], str):
            args = (rewrite_combined_pdf_html(args[0]),) + args[1:]
        return orig_html_cls(*args, **kwargs)

    def _collect_readme_pages(node) -> list[str]:
        found: list[str] = []
        if isinstance(node, list):
            for item in node:
                found.extend(_collect_readme_pages(item))
        elif isinstance(node, dict):
            for val in node.values():
                found.extend(_collect_readme_pages(val))
        elif isinstance(node, str) and posixpath.basename(node) == "README.md":
            found.append(node)
        return found

    def _PatchedGeneratePDF(site_dir: str, mkdocs_yml_path: str, pdf_output_path: str) -> None:
        temp_copies: list[str] = []
        try:
            with open(mkdocs_yml_path, "r", encoding="utf-8") as fh:
                cfg = yaml.safe_load(fh) or {}
            for rel_md in _collect_readme_pages(cfg.get("nav", [])):
                rel_dir = posixpath.dirname(rel_md)
                site_sub = os.path.join(site_dir, rel_dir) if rel_dir else site_dir
                idx_html = os.path.join(site_sub, "index.html")
                readme_html = os.path.join(site_sub, "README.html")
                if os.path.isfile(idx_html) and not os.path.exists(readme_html):
                    shutil.copy2(idx_html, readme_html)
                    temp_copies.append(readme_html)
            orig_generate_pdf(site_dir, mkdocs_yml_path, pdf_output_path)
        finally:
            for tmp_path in temp_copies:
                if os.path.exists(tmp_path):
                    os.remove(tmp_path)

    mk_pdf.HTML = _PatchedHTML
    mk_pdf.generate_pdf = _PatchedGeneratePDF
    mk_cli.generate_pdf = _PatchedGeneratePDF
    sys.argv[0] = "mkdocs-kit"
    return mk_cli.main() or 0


if __name__ == "__main__":
    sys.exit(main())
