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

PROJECT_ROOT_DIR=$(cd "$SCRIPT_DIR" && cd .. && pwd)
echo "PROJECT_ROOT_DIR: $PROJECT_ROOT_DIR"

GEMINI_MODEL_LVL0="gemini-2.5-flash-lite"
GEMINI_MODEL_LVL1="gemini-2.5-flash"
GEMINI_MODEL_LVL2="gemini-3-flash-preview"
GEMINI_MODEL_LVL3="gemini-2.5-pro"
GEMINI_MODEL_LVL4="gemini-3-pro-preview"


OPENCODE_MODEL_LVL0="ollama/ralph-opencode-v1"
OPENCODE_MODEL_LVL1="ollama/ralph-opencode-v1"
OPENCODE_MODEL_LVL2="ollama/ralph-opencode-v1"
OPENCODE_MODEL_LVL3="ollama/ralph-opencode-v1"
OPENCODE_MODEL_LVL4="ollama/ralph-opencode-v1"

ENUM_CLI_GEMINI="gemini"
ENUM_CLI_OPENCODE="opencode"

USED_CLI="$ENUM_CLI_OPENCODE"

iterateOverCmakeProjects()
{
    find "$PROJECT_ROOT_DIR/cmake-projects" -maxdepth 1 -mindepth 1 -type d 
}

invokeGemini()
{
    local workDir="$1"
    local cGeminiModel="$2" 
    local cGemniPrompt="$3"
    echo "PWD: $PWD"
    echo "workDir: $workDir"
    echo "cGeminiModel: $cGeminiModel"
    echo "cGemniPrompt: $cGemniPrompt"
    
    pushd $PWD
    cd $workDir
    gemini -y -m "$cGeminiModel" -p "$cGemniPrompt"
    popd
}

invokeOpenCode()
{
    local workDir="$1"
    local cOpenCodeModel="$2" 
    local cOpenCodePrompt="$3"
    echo "PWD: $PWD"
    echo "workDir: $workDir"
    echo "cOpenCodeModel: $cOpenCodeModel"
    echo "cOpenCodePrompt: $cOpenCodePrompt"
    
    pushd $PWD
    cd $workDir
    opencode --agent "build" --model "$cOpenCodeModel" --prompt "$cOpenCodePrompt"
    popd
}

getModelNameFromLevel()
{
    local modelLevel="$1"
    if [ "$USED_CLI" == "gemini" ]; then
        if [ "$modelLevel" == "0" ]; then
            echo "$GEMINI_MODEL_LVL0"
        elif [ "$modelLevel" == "1" ]; then
            echo "$GEMINI_MODEL_LVL1"
        elif [ "$modelLevel" == "2" ]; then
            echo "$GEMINI_MODEL_LVL2"
        elif [ "$modelLevel" == "3" ]; then
            echo "$GEMINI_MODEL_LVL3"
        elif [ "$modelLevel" == "4" ]; then
            echo "$GEMINI_MODEL_LVL4"
        fi
    elif [ "$USED_CLI" == "opencode" ]; then
        if [ "$modelLevel" == "0" ]; then
            echo "$OPENCODE_MODEL_LVL0"
        elif [ "$modelLevel" == "1" ]; then
            echo "$OPENCODE_MODEL_LVL1"
        elif [ "$modelLevel" == "2" ]; then
            echo "$OPENCODE_MODEL_LVL2"
        elif [ "$modelLevel" == "3" ]; then
            echo "$OPENCODE_MODEL_LVL3"
        elif [ "$modelLevel" == "4" ]; then
            echo "$OPENCODE_MODEL_LVL4"
        fi
    fi
}


invokeCli()
{
    if [ "$USED_CLI" == "gemini" ]; then
        invokeGemini "$1" "$2" "$3"
    elif [ "$USED_CLI" == "opencode" ]; then
        invokeOpenCode "$1" "$2" "$3"
    fi
}


checkConstraintsComplianceImpl()
{
    local currentCmakeProjectDir="$1"
    echo "currentCmakeProjectDir: $currentCmakeProjectDir"

    for cConstraintFile in $(find $PROJECT_ROOT_DIR/doc/architecture -type f -name "CS*md")
    {
        echo "PWD: $PWD"
        pushd $PWD
        cd $PROJECT_ROOT_DIR
        echo "PWD: $PWD"


        echo "cConstraintFile: $cConstraintFile"
        local constraintFileRelativePath="$(getRelativePath $PROJECT_ROOT_DIR $cConstraintFile)"
        local currentCmakeProjectDirRelativePath="$(getRelativePath $PROJECT_ROOT_DIR $currentCmakeProjectDir)"
        echo "constraintFileRelativePath: $constraintFileRelativePath"
        echo "currentCmakeProjectDirRelativePath: $currentCmakeProjectDirRelativePath"

        local cAgentPrompt="
        Your goal is to identify in ${currentCmakeProjectDirRelativePath} the contribution of the code regarding features described in ${constraintFileRelativePath}.

        In order to do that, you will have to:
        1. Read the content of ${constraintFileRelativePath} and display it.
            1.1 Identify the constraints described in ${constraintFileRelativePath}.
            1.2 Display your understanding of each constraint.
            1.3 Explain how the conformance of the code to each constraint will be verified.
        2. Read the content of ${currentCmakeProjectDirRelativePath}.
            2.1 Analyze project's structure, beginning with CMAKELists.txt files.
            2.2 Identify structures, classes, methods, files, etc.
            2.3 Obtain an understanding of the code, its goals, its architecture, its design, etc.
            2.4 Obtain an understanding of the execution flow of the code.
            2.5 During the process, display all ongoing thinking.
        3. For each constraint, explain how the code conforms to it.
        4. For each constraint, explain how the code does not conform to it.
        5. For each constraint, explain how the code could be modified to conform to it.
        6. At the end of the process, report all done modifications.
        7. IT IS FORBIDDEN TO ADD, MODIFY OR REMOVE ANY PROJECT FILES.
        "

        invokeCli "$PROJECT_ROOT_DIR" "$(getModelNameFromLevel 2)" "$cAgentPrompt"
        echo "PWD: $PWD"
        popd
        echo "PWD: $PWD"
    }

    

}


proceedWithCheckConstraintsCompliance()
{
       for cmakeProjectDir in $(iterateOverCmakeProjects); do
           checkConstraintsComplianceImpl "$cmakeProjectDir"
       done
}

proceedWithCheckConstraintsCompliance



