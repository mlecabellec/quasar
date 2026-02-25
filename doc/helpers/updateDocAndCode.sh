#!/bin/bash

getScriptDir()
{
    local SOURCE_PATH="${BASH_SOURCE[0]}"
    local SYMLINK_DIR
    local SCRIPT_DIR
    # Resolve symlinks recursively
    while [ -L "$SOURCE_PATH" ]; do
        # Get symlink directory
        SYMLINK_DIR="$( cd -P "$( dirname "$SOURCE_PATH" )" >/dev/null 2>&1 && pwd )"
        # Resolve symlink target (relative or absolute)
        SOURCE_PATH="$(readlink "$SOURCE_PATH")"
        # Check if candidate path is relative or absolute
        if [[ $SOURCE_PATH != /* ]]; then
            # Candidate path is relative, resolve to full path
            SOURCE_PATH=$SYMLINK_DIR/$SOURCE_PATH
        fi
    done
    # Get final script directory path from fully resolved source path
    SCRIPT_DIR="$(cd -P "$( dirname "$SOURCE_PATH" )" >/dev/null 2>&1 && pwd)"
    echo "$SCRIPT_DIR"
}

SCRIPT_DIR="$(getScriptDir)"
echo "SCRIPT_DIR: $SCRIPT_DIR"

PROJECT_ROOT_DIR=$(cd "$SCRIPT_DIR" && cd .. && cd .. && pwd)
echo "PROJECT_ROOT_DIR: $PROJECT_ROOT_DIR"

GEMINI_MODEL_LVL0="gemini-2.5-flash-lite"
GEMINI_MODEL_LVL1="gemini-2.5-flash"
GEMINI_MODEL_LVL2="gemini-3-flash-preview"
GEMINI_MODEL_LVL3="gemini-2.5-pro"
GEMINI_MODEL_LVL4="gemini-3-pro-preview"


iterateOverCmakeProjects()
{
    find "$PROJECT_ROOT_DIR/cmake-projects" -maxdepth 1 -mindepth 1 -type d 
}


updateContributionToFeaturesImpl()
{
    local currentCmakeProjectDir="$1"
    echo "currentCmakeProjectDir: $currentCmakeProjectDir"

    for cFeatureFile in $(find $PROJECT_ROOT_DIR/doc/features -type f -name "FE*md")
    {
        echo "cFeatureFile: $cFeatureFile"
        local cGemniPrompt="
        Your goal is to identify in @${currentCmakeProjectDir} the contribution of the code regarding features described in @${cFeatureFile}.

        In order to do that, you will have to:
        1. Read the content of @${cFeatureFile} and display it.
        2. Read the content of @${currentCmakeProjectDir}.
            2.1 Identify structures, classes, methods, files, etc.
            2.2 Obtain an understanding of the code, its goals, its architecture, its design, etc.
            2.3 Obtain an understanding of the execution flow of the code.
            2.4 During the process, display all ongoing thinking.
        3. Identify the contribution of the code regarding features described in @${cFeatureFile}.
        4. Create or update comment in @${currentCmakeProjectDir} to document the contribution of the code regarding features described in @${cFeatureFile}.
            4.1 Create or update doxygen comments. Create or update comment inside method bodies.
            4.2 The added or updated comment shall cite the reference of the feature.
            4.3 The added or updated comment shall explain how the commented code contributes to the feature.
            4.4 The added or updated comment shall provide an argumentation of the contribution.
            4.5 If the contribution of the code does not fully cover the feature, the added or updated comment shall explain the missing part.
            4.6 During the process, display all ongoing thinking.
        "

        gemini -y -m "$GEMINI_MODEL_LVL0" -p "$cGemniPrompt"
    }

    

}


proceedWithFeatureContributionUpdate()
{
       for cmakeProjectDir in $(iterateOverCmakeProjects); do
           updateContributionToFeaturesImpl "$cmakeProjectDir"
       done
}

proceedWithFeatureContributionUpdate



