import os
import urllib.parse
from html.parser import HTMLParser

class LinkParser(HTMLParser):
    def __init__(self):
        super().__init__()
        self.links = []

    def handle_starttag(self, tag, attrs):
        attrs_dict = dict(attrs)
        if tag == 'a' and 'href' in attrs_dict:
            self.links.append(('a', attrs_dict['href']))
        elif tag == 'img' and 'src' in attrs_dict:
            self.links.append(('img', attrs_dict['src']))
        elif tag == 'link' and 'href' in attrs_dict:
            self.links.append(('link', attrs_dict['href']))
        elif tag == 'script' and 'src' in attrs_dict:
            self.links.append(('script', attrs_dict['src']))

def check_links():
    site_dir = 'doc/site'
    if not os.path.exists(site_dir):
        print(f"Error: {site_dir} does not exist. Run build first.")
        return False

    broken = 0
    total = 0

    for root, dirs, files in os.walk(site_dir):
        for file in files:
            if not file.endswith('.html'):
                continue
            
            filepath = os.path.join(root, file)
            with open(filepath, 'r', encoding='utf-8', errors='ignore') as f:
                content = f.read()

            parser = LinkParser()
            parser.feed(content)

            for tag, href in parser.links:
                # Ignore external links, mailto, javascript, local anchors, and direct file:/// references
                if href.startswith(('http://', 'https://', 'mailto:', 'javascript:', '#', 'file:')):
                    continue
                
                # Strip anchor and query parameters
                url_parsed = urllib.parse.urlparse(href)
                path = url_parsed.path
                if not path:
                    continue
                
                # Decode url-encoded paths (e.g. %20)
                path = urllib.parse.unquote(path)

                total += 1
                
                # Resolve target file path relative to current html file
                # If path starts with /, it's relative to the site_dir root
                if path.startswith('/'):
                    target_path = os.path.join(site_dir, path.lstrip('/'))
                else:
                    target_path = os.path.join(root, path)

                # MkDocs clean URLs maps pages to directories with index.html.
                # Since use_directory_urls is false, links should be direct html files.
                # However, if target_path is still a directory, check for index.html.
                if os.path.isdir(target_path):
                    target_path = os.path.join(target_path, 'index.html')

                if not os.path.exists(target_path):
                    print(f"Broken link in {filepath}: {tag} target '{href}' (resolved to '{target_path}') not found!")
                    broken += 1

    print(f"\nLink check complete: {total} local links checked, {broken} broken links found.")
    return broken == 0

if __name__ == '__main__':
    import sys
    success = check_links()
    sys.exit(0 if success else 1)
