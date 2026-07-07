#!/bin/bash
# REQ-00026 – Offline build support / clean navigation
# TSK-00306 – Documentation helper builder script using mkdocs-kit

# Safe mode flags
set -euo pipefail

# Determine script and root directories
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ROOT_DIR="$(cd "$SCRIPT_DIR/.." && pwd)"

# Color definitions
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[0;33m'
BLUE='\033[0;34m'
NC='\033[0m' # Reset Color

info() {
    echo -e "${BLUE}[INFO]${NC} $1"
}

success() {
    echo -e "${GREEN}[SUCCESS]${NC} $1"
}

warning() {
    echo -e "${YELLOW}[WARNING]${NC} $1"
}

error() {
    echo -e "${RED}[ERROR]${NC} $1" >&2
}

show_help() {
    cat << EOF
Usage: $(basename "$0") [command] [options]

Robust documentation builder script relying on mkdocs-kit to compile HTML, PDF, and Man page manuals.

Commands:
  build         Build the documentation site and PDF (default)
  serve         Start a local development server for live documentation preview
  compile       Compile PlantUML wireframe diagrams to PNG in the output site directory
  clean         Remove generated site, PDF, and temporary outputs

Options:
  --refresh     Force pull/update of the cached mkdocs-kit repository
  -h, --help    Show this help message
EOF
}

# Defaults
COMMAND="build"
REFRESH=false

# Parse arguments
while [[ $# -gt 0 ]]; do
    case "$1" in
        -h|--help)
            show_help
            exit 0
            ;;
        --refresh)
            REFRESH=true
            shift
            ;;
        build|serve|compile|clean)
            COMMAND="$1"
            shift
            ;;
        *)
            error "Unknown command or option: $1"
            show_help
            exit 1
            ;;
    esac
done

# Define cache directories in the workspace
CACHE_DIR="$ROOT_DIR/.cache"
MKDOCS_KIT_DIR="$CACHE_DIR/mkdocs-kit"
MKDOCS_KIT_VENV="$MKDOCS_KIT_DIR/.venv"
MKDOCS_KIT_CLI="$MKDOCS_KIT_DIR/src/mkdocs_kit/cli.py"

# Precondition checks
check_dependencies() {
    info "Verifying dependencies..."
    
    # Check if python3 is available
    if ! command -v python3 &>/dev/null; then
        error "python3 is not installed or not in PATH."
        exit 1
    fi
    
    # Check if git is available
    if ! command -v git &>/dev/null; then
        error "git is not installed or not in PATH."
        exit 1
    fi
    
    # Check if plantuml is available
    if ! command -v plantuml &>/dev/null; then
        error "plantuml command is not installed or not in PATH."
        exit 1
    fi
    
    success "All system dependencies verified."
}

# Resolve and cache mkdocs-kit
setup_mkdocs_kit() {
    check_dependencies
    
    mkdir -p "$CACHE_DIR"
    
    if [[ ! -d "$MKDOCS_KIT_DIR/.git" ]]; then
        info "mkdocs-kit not found in cache. Cloning from GitHub..."
        git clone https://github.com/mlecabellec/mkdocs-kit.git "$MKDOCS_KIT_DIR"
        success "Cloned mkdocs-kit to $MKDOCS_KIT_DIR."
    else
        info "Found cached mkdocs-kit repository."
        if [ "$REFRESH" = true ]; then
            info "Refreshing cached repository from GitHub..."
            cd "$MKDOCS_KIT_DIR"
            git pull
            cd - >/dev/null
            success "Successfully updated mkdocs-kit repository."
        fi
    fi
    
    # Setup virtual environment
    if [[ ! -d "$MKDOCS_KIT_VENV" ]]; then
        info "Creating virtual environment for mkdocs-kit..."
        python3 -m venv "$MKDOCS_KIT_VENV"
        
        info "Installing Python dependencies (this might take a few moments)..."
        "$MKDOCS_KIT_VENV/bin/pip" install --upgrade pip
        "$MKDOCS_KIT_VENV/bin/pip" install "setuptools<82.0.0"
        "$MKDOCS_KIT_VENV/bin/pip" install mkdocs mkdocs-material weasyprint wireviz nwdiag bit_field
        success "Virtual environment setup complete."
    else
        info "Using cached Python virtual environment."
    fi
}

compile_diagrams() {
    info "Compiling PlantUML diagrams to PNG in target site output..."
    local img_dir="$ROOT_DIR/doc/docs/architecture"
    local out_dir="$ROOT_DIR/doc/site/architecture/images"
    
    if [[ ! -d "$img_dir" ]]; then
        error "Source architecture directory not found at '$img_dir'."
        exit 1
    fi
    
    mkdir -p "$out_dir"
    
    # Compile diagrams to target site output directory
    if plantuml -tpng -o "$out_dir" "$img_dir"/*.puml; then
        success "All wireframe diagrams successfully compiled to: $out_dir"
    else
        error "Failed to compile PlantUML wireframe diagrams."
        exit 1
    fi
}

clean_docs() {
    info "Cleaning documentation build artifacts..."
    
    # Remove generated site directory
    if [[ -d "$ROOT_DIR/doc/site" ]]; then
        rm -rf "$ROOT_DIR/doc/site"
        info "Removed site/ directory."
    fi
    
    # Remove generated PDF
    if [[ -f "$ROOT_DIR/doc/documentation.pdf" ]]; then
        rm -f "$ROOT_DIR/doc/documentation.pdf"
        info "Removed documentation.pdf."
    fi
    
    # Clean any accidental PNG files in source directory
    local img_dir="$ROOT_DIR/doc/docs/architecture"
    if [[ -d "$img_dir" ]]; then
        find "$img_dir" -name "*.png" -delete
    fi
    
    success "Cleanup complete."
}

# Main routing logic
case "$COMMAND" in
    clean)
        clean_docs
        ;;
    compile)
        setup_mkdocs_kit
        compile_diagrams
        ;;
    build)
        setup_mkdocs_kit
        
        info "Building documentation with mkdocs-kit..."
        cd "$ROOT_DIR/doc"
        
        PYTHONPATH="$MKDOCS_KIT_DIR/src" "$MKDOCS_KIT_VENV/bin/python3" "$MKDOCS_KIT_CLI" build
        
        success "Documentation successfully built at: $ROOT_DIR/doc/site/"
        ;;
    serve)
        setup_mkdocs_kit
        
        info "Serving live documentation with mkdocs-kit..."
        cd "$ROOT_DIR/doc"
        
        PYTHONPATH="$MKDOCS_KIT_DIR/src" "$MKDOCS_KIT_VENV/bin/python3" "$MKDOCS_KIT_CLI" serve
        ;;
esac
