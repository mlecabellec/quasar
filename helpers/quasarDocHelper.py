#!/usr/bin/env python3
import argparse
import os
import glob
import subprocess
import sys
import re
from pathlib import Path

# Constants
PROJECT_ROOT = Path(__file__).resolve().parent.parent.parent
FEATURES_DIR = PROJECT_ROOT / "doc" / "features"
CMAKE_PROJECTS_DIR = PROJECT_ROOT / "cmake-projects"
EXCLUDED_PROJECTS = ["third-party"]

class QuasarManager:
    def __init__(self, yolo=False, model="gemini-3-flash-preview"):
        self.yolo = yolo
        self.model = model

    def get_features(self):
        """Extracts feature IDs and titles from FE-*.md files."""
        features = {}
        feature_files = glob.glob(str(FEATURES_DIR / "FE-*.md"))
        for f in sorted(feature_files):
            with open(f, 'r') as file:
                content = file.read()
                # Find the main feature ID and title
                main_id_match = re.search(r'# Feature (\d+)', content)
                title_match = re.search(r'## Title\n\n(.*?)\n', content)
                
                if main_id_match:
                    main_id = f"FE-{main_id_match.group(1)}"
                    # Find all sub-features [FE-XXXX.Y.Z]
                    sub_features = re.findall(r'\[(FE-\d+\.\d+(?:\.\d+)?)\]\s+(.*?)(?=\n| - | \[|$)', content)
                    features[main_id] = {
                        'title': title_match.group(1) if title_match else main_id,
                        'sub_features': sub_features
                    }
        return features

    def get_projects(self):
        """Returns a list of project directories in cmake-projects, excluding third-party."""
        projects = []
        if not CMAKE_PROJECTS_DIR.exists():
            return []
        for d in CMAKE_PROJECTS_DIR.iterdir():
            if d.is_dir() and d.name not in EXCLUDED_PROJECTS:
                projects.append(d)
        return sorted(projects)

    def task_doc_features(self):
        """Invokes Gemini CLI to document methods and code fulfilling requested features."""
        features = self.get_features()
        projects = self.get_projects()
        
        print(f"--- Found {len(features)} feature sets and {len(projects)} projects ---")

        for project in projects:
            project_name = project.name
            print(f"\n>>> Analyzing project: {project_name}")
            
            # Prepare feature context for the prompt
            relevant_features = []
            for fid, fdoc in features.items():
                if project_name.lower() in str(fdoc).lower() or project_name.lower() in fid.lower():
                     relevant_features.append((fid, fdoc))
            
            if not relevant_features:
                relevant_features = list(features.items())

            # Construct the Gemini prompt
            feature_context = "\n".join([f"- {fid}: {fdoc['title']}\n  " + 
                                         "\n  ".join([f"  * {sid}: {sdesc}" for sid, sdesc in fdoc['sub_features']])
                                         for fid, fdoc in relevant_features])

            prompt = f"""
I am currently analyzing the C++ project '{project_name}' located in '{project.relative_to(PROJECT_ROOT)}'.
I need you to review the source code (header and implementation files) and add or update comments to document methods or code lines that fulfill the following requested features:

{feature_context}

CRITICAL CONSTRAINTS:
1. You MUST NOT modify functional code.
2. You CAN ONLY add or update comments.
3. Every comment documenting a feature MUST cite the related feature ID (e.g., [FE-0010.1.1]).
4. Focus on documenting methods, classes, and implemented sequences.
5. Use the project's existing commenting style.
6. Only process files within '{project.relative_to(PROJECT_ROOT)}'.
7. Do not summarize your changes; just apply them surgically.
"""
            # Command to invoke Gemini CLI
            gemini_cmd = ["gemini", "--model", self.model, "-p", prompt]
            if self.yolo:
                gemini_cmd.append("-y")

            print(f"Invoking Gemini CLI for {project_name} (model: {self.model})...")
            try:
                subprocess.run(gemini_cmd, check=True)
            except subprocess.CalledProcessError as e:
                print(f"Error invoking Gemini CLI for {project_name}: {e}")
                sys.exit(1)
            except FileNotFoundError:
                print("Error: 'gemini' command not found in PATH.")
                sys.exit(1)

def main():
    parser = argparse.ArgumentParser(description="Quasar Project Management Tool")
    parser.add_argument("--yolo", action="store_true", help="Enable YOLO mode (minimal confirmation)")
    parser.add_argument("--model", default="gemini-3-flash-preview", help="Gemini model to use")
    subparsers = parser.add_subparsers(dest="command", help="Available commands")

    # Task: doc-features
    subparsers.add_parser("doc-features", help="Analyze code and add feature documentation comments")

    args = parser.parse_args()

    manager = QuasarManager(yolo=args.yolo, model=args.model)

    if args.command == "doc-features":
        manager.task_doc_features()
    else:
        parser.print_help()

if __name__ == "__main__":
    main()
