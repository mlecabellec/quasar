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


getRelativePath() {
    local source="$1"
    local target="$2"
    
    local common="$source"
    local result=""

    # 1. Trouver la base commune
    while [[ "${target#$common}" == "$target" ]]; do
        common=$(dirname "$common")
        result="../$result"
    done

    # 2. Ajouter la partie spécifique de la cible
    local forward_part="${target#$common}"
    
    # Nettoyage des slashs
    forward_part="${forward_part#/}"
    
    echo "${result}${forward_part}"
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
        echo "PWD: $PWD"
        pushd $PWD
        cd $PROJECT_ROOT_DIR
        echo "PWD: $PWD"


        echo "cFeatureFile: $cFeatureFile"
        local featureFileRelativePath="$(getRelativePath $PROJECT_ROOT_DIR $cFeatureFile)"
        local currentCmakeProjectDirRelativePath="$(getRelativePath $PROJECT_ROOT_DIR $currentCmakeProjectDir)"
        echo "featureFileRelativePath: $featureFileRelativePath"
        echo "currentCmakeProjectDirRelativePath: $currentCmakeProjectDirRelativePath"

        local cGemniPrompt="
        Your goal is to identify in @${currentCmakeProjectDirRelativePath} the contribution of the code regarding features described in @${featureFileRelativePath}.

        In order to do that, you will have to:
        1. Read the content of @${featureFileRelativePath} and display it.
        2. Read the content of @${currentCmakeProjectDirRelativePath}.
            2.1 Identify structures, classes, methods, files, etc.
            2.2 Obtain an understanding of the code, its goals, its architecture, its design, etc.
            2.3 Obtain an understanding of the execution flow of the code.
            2.4 During the process, display all ongoing thinking.
        3. Identify the contribution of the code regarding features described in @${featureFileRelativePath}.
        4. Create or update comment in @${currentCmakeProjectDirRelativePath} to document the contribution of the code regarding features described in @${featureFileRelativePath}.
            4.1 YOU SHALL ONLY ADD OR MODIFY COMMENTS. IT IS FORBIDDEN TO MODIFY FUNCTIONAL CODE.
            4.1 Create or update doxygen comments. Create or update comment inside method bodies.
            4.2 The added or updated comment shall cite the reference of the feature.
            4.3 The added or updated comment shall explain how the commented code contributes to the feature.
            4.4 The added or updated comment shall provide an argumentation of the contribution.
            4.5 If the contribution of the code does not fully cover the feature, the added or updated comment shall explain the missing part.
            4.6 During the process, display all ongoing thinking.
        5. Analyze specifically tests and verify if tests are consistent with @${featureFileRelativePath}.
            5.1 Verify that the implemented test sequence prove formally the contribution and the conformance to the feature. Comment shall explain how the tests prove the contribution and the conformance.
            5.2 If the tests are not consistent, update them. If contribution is not proven, add \"TO BE CONFIRMED\" to the comment.
            5.3 During the process, display all ongoing thinking.
        6. After creation or update, make an additional analysis on the comments and verify if its still consistent with @${featureFileRelativePath}.
            6.1 If the comments are not consistent, update them. If contribution is not proven, add \"TO BE CONFIRMED\" to the comment.
            6.2 During the process, display all ongoing thinking.
        7. At the end of the process, report all done modifications.
        8. YOU SHALL ONLY ADD OR MODIFY COMMENTS. IT IS FORBIDDEN TO MODIFY FUNCTIONAL CODE.
        9. IT IS FORBIDDEN TO ADD, MODIFY OR REMOVE ANY CODE. ONLY COMMENTS ARE ALLOWED.
        10. IT IS FORBIDDEN TO ADD OR REMOVE ANY PROJECT FILES.
        11. IT IS FORBIDDEN TO MODIFY PROJECTS FILES LIKE CMakeLists.txt, etc.
        12. For each planned modification, cross check your own work. If quality of the modification is not proven, don't do it.
        "

        gemini -y -m "$GEMINI_MODEL_LVL0" -p "$cGemniPrompt"
        echo "PWD: $PWD"
        popd
        echo "PWD: $PWD"
    }

    

}


proceedWithFeatureContributionUpdate()
{
       for cmakeProjectDir in $(iterateOverCmakeProjects); do
           updateContributionToFeaturesImpl "$cmakeProjectDir"
       done
}

proceedWithFeatureContributionUpdate



