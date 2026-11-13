#!/usr/bin/env python3
"""
Validates Quasar documentation consistency:
1. All links in built HTML (doc/site) resolve to valid local files (no directory links, no file://, no leading /).
2. All Markdown links in doc/docs/*.md use valid relative file paths (no file://, no /home/..., no directory links).
3. Navigation tree in doc/mkdocs.yml is consistent (all nav targets exist and every non-internal doc page is reachable).
"""

import os
import pathlib
import posixpath
import re
import sys
import urllib.parse
from html.parser import HTMLParser
import yaml


_FENCE_RE = re.compile(r"^```")
_MD_LINK_RE = re.compile(r"!?\[[^\]]*\]\(([^)]+)\)")


class LinkParser(HTMLParser):
    def __init__(self):
        super().__init__()
        self.links = []

    def handle_starttag(self, tag, attrs):
        attrs_dict = dict(attrs)
        if tag == "a" and "href" in attrs_dict:
            self.links.append(("a", attrs_dict["href"]))
        elif tag == "img" and "src" in attrs_dict:
            self.links.append(("img", attrs_dict["src"]))
        elif tag == "link" and "href" in attrs_dict:
            self.links.append(("link", attrs_dict["href"]))
        elif tag == "script" and "src" in attrs_dict:
            self.links.append(("script", attrs_dict["src"]))


def check_html_links(site_dir: str = "doc/site") -> bool:
    if not os.path.exists(site_dir):
        print(f"Error: {site_dir} does not exist. Run build first.")
        return False

    broken = 0
    total = 0

    for root, _dirs, files in os.walk(site_dir):
        for file in files:
            if not file.endswith(".html") or file == "404.html":
                continue

            filepath = os.path.join(root, file)
            with open(filepath, "r", encoding="utf-8", errors="ignore") as f:
                content = f.read()

            parser = LinkParser()
            parser.feed(content)

            for tag, href in parser.links:
                if href.startswith(("http://", "https://", "mailto:", "javascript:", "#")):
                    continue

                total += 1
                if href.startswith("file:"):
                    print(f"Forbidden file:// link in {filepath}: {tag} href='{href}'")
                    broken += 1
                    continue

                url_parsed = urllib.parse.urlparse(href)
                path = urllib.parse.unquote(url_parsed.path)
                if not path:
                    continue

                if tag == "a" and path.startswith("/"):
                    print(f"Forbidden absolute path link in {filepath}: {tag} href='{href}'")
                    broken += 1
                    continue

                if path.startswith("/"):
                    target_path = os.path.join(site_dir, path.lstrip("/"))
                else:
                    target_path = os.path.normpath(os.path.join(root, path))

                if tag == "a" and (os.path.isdir(target_path) or path.endswith("/")):
                    print(f"Forbidden directory link in {filepath}: {tag} href='{href}' -> {target_path}")
                    broken += 1
                    continue

                if not os.path.exists(target_path):
                    print(f"Broken link in {filepath}: {tag} target '{href}' (resolved to '{target_path}') not found!")
                    broken += 1

    print(f"HTML check complete: {total} local links checked, {broken} issues found.")
    return broken == 0


def check_markdown_sources(docs_dir: str = "doc/docs", mkdocs_yml: str = "doc/mkdocs.yml") -> bool:
    docs_root = pathlib.Path(docs_dir).resolve()
    md_files = sorted(
        p for p in docs_root.rglob("*.md")
        if not any(part.startswith(".") for part in p.relative_to(docs_root).parts)
    )

    md_issues = 0
    md_links = 0

    for md_path in md_files:
        in_fence = False
        text = md_path.read_text(encoding="utf-8", errors="replace")
        for lineno, raw_line in enumerate(text.splitlines(), 1):
            stripped = raw_line.lstrip()
            if _FENCE_RE.match(stripped):
                in_fence = not in_fence
                continue
            if in_fence:
                continue

            # Ignore inline code spans when scanning markdown links
            line_no_code = re.sub(r"`[^`]*`", "", raw_line)
            # Handle balanced parentheses inside filenames like foo(bar).pdf when scanning links
            for m in re.finditer(r"!?\[[^\]]*\]\(", line_no_code):
                start = m.end()
                depth = 1
                pos = start
                while pos < len(line_no_code) and depth > 0:
                    ch = line_no_code[pos]
                    if ch == "(":
                        depth += 1
                    elif ch == ")":
                        depth -= 1
                    pos += 1
                if depth != 0:
                    continue
                target_raw = line_no_code[start : pos - 1].strip()
                if not target_raw:
                    continue
                href = target_raw.split()[0].strip("<>")
                if href.startswith(("http://", "https://", "mailto:", "javascript:", "#")):
                    continue

                md_links += 1
                rel_md = md_path.relative_to(docs_root)
                if href.startswith(("file:", "/")):
                    print(f"Forbidden absolute/file link in {rel_md}:{lineno} -> '{href}'")
                    md_issues += 1
                    continue

                path_part, _frag = urllib.parse.urldefrag(href)
                if not path_part:
                    continue
                dec_part = urllib.parse.unquote(path_part)
                resolved = (md_path.parent / dec_part).resolve()

                if not resolved.exists():
                    print(f"Broken markdown link in {rel_md}:{lineno} -> '{href}'")
                    md_issues += 1
                elif resolved.is_dir() or dec_part.endswith("/"):
                    print(f"Forbidden directory markdown link in {rel_md}:{lineno} -> '{href}'")
                    md_issues += 1

    print(f"Markdown source check complete: {md_links} links checked across {len(md_files)} files, {md_issues} issues found.")

    # Navigation tree consistency & completeness check
    nav_issues = 0
    with open(mkdocs_yml, "r", encoding="utf-8") as f:
        cfg = yaml.safe_load(f) or {}

    nav_files = set()

    def _walk_nav(node):
        if isinstance(node, dict):
            for val in node.values():
                _walk_nav(val)
        elif isinstance(node, list):
            for item in node:
                _walk_nav(item)
        elif isinstance(node, str):
            nav_files.add(posixpath.normpath(node))

    _walk_nav(cfg.get("nav", []))

    for nav_f in sorted(nav_files):
        if nav_f.startswith("/") or ".." in pathlib.PurePosixPath(nav_f).parts:
            print(f"Invalid nav path in {mkdocs_yml}: '{nav_f}'")
            nav_issues += 1
        target = docs_root / nav_f
        if not target.exists():
            print(f"Missing nav target in {mkdocs_yml}: '{nav_f}'")
            nav_issues += 1
        elif target.is_dir():
            print(f"Forbidden directory nav entry in {mkdocs_yml}: '{nav_f}'")
            nav_issues += 1

    all_rel_mds = {str(p.relative_to(docs_root).as_posix()) for p in md_files}
    unlinked = sorted(all_rel_mds - nav_files)
    if unlinked:
        print(f"Unlinked markdown pages not reachable from {mkdocs_yml} nav ({len(unlinked)}):")
        for u in unlinked[:30]:
            print(f"  - {u}")
        if len(unlinked) > 30:
            print(f"  ... and {len(unlinked) - 30} more")
        nav_issues += len(unlinked)

    print(f"Navigation check complete: {len(nav_files)} nav entries, {len(all_rel_mds)} markdown pages, {nav_issues} issues.")
    return md_issues == 0 and nav_issues == 0


def check_links() -> bool:
    html_ok = check_html_links()
    md_ok = check_markdown_sources()
    return html_ok and md_ok


if __name__ == "__main__":
    success = check_links()
    sys.exit(0 if success else 1)
