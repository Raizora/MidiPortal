#!/bin/bash
set -e

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[0;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

echo -e "${BLUE}=== MidiPortal Build Script ===${NC}"

# Check for Ninja
if ! command -v ninja &> /dev/null; then
    echo -e "${RED}Ninja build system not found!${NC}"
    echo -e "${YELLOW}Installing Ninja via Homebrew...${NC}"
    brew install ninja
fi

# Check for Clang
if ! command -v clang &> /dev/null; then
    echo -e "${RED}Clang compiler not found!${NC}"
    echo -e "${YELLOW}Please install Xcode command line tools with:${NC}"
    echo "xcode-select --install"
    exit 1
fi

# Parse command line arguments
BUILD_PLUGIN=false
BUILD_TYPE="Debug"

for arg in "$@"; do
    case $arg in
        --plugin)
            BUILD_PLUGIN=true
            shift
            ;;
        --release)
            BUILD_TYPE="Release"
            shift
            ;;
        *)
            # Unknown option
            ;;
    esac
done

# Build Rust library first
echo -e "${BLUE}Building Rust library...${NC}"
(cd rust && cargo build --release)

# Clean build directory
echo -e "${BLUE}Cleaning build directory...${NC}"
rm -rf build

# Configure with CMake using Ninja and Apple Clang
echo -e "${BLUE}Configuring with CMake...${NC}"
cmake -B build -G Ninja \
      -DCMAKE_C_COMPILER=/usr/bin/clang \
      -DCMAKE_CXX_COMPILER=/usr/bin/clang++ \
      -DCMAKE_BUILD_TYPE=$BUILD_TYPE \
      -DCMAKE_EXPORT_COMPILE_COMMANDS=ON

# Build the project
echo -e "${BLUE}Building project...${NC}"

if [ "$BUILD_PLUGIN" = true ]; then
    # Build everything including plugin
    echo -e "${YELLOW}Building with plugin support...${NC}"
    
    # Check for juce_vst3_helper
    if ! command -v juce_vst3_helper &> /dev/null; then
        echo -e "${YELLOW}Warning: juce_vst3_helper not found in PATH${NC}"
        echo -e "${YELLOW}Attempting to find it in JUCE directory...${NC}"
        
        # Try to find juce_vst3_helper in common locations
        JUCE_VST3_HELPER=""
        for path in \
            "/Users/jbenchia/JUCE/extras/Build/juceaide/build/Debug/juce_vst3_helper" \
            "/Users/jbenchia/JUCE/extras/Build/juceaide/build/Release/juce_vst3_helper" \
            "$(find /Users/jbenchia/JUCE -name "juce_vst3_helper" -type f 2>/dev/null | head -n 1)"
        do
            if [ -f "$path" ]; then
                JUCE_VST3_HELPER="$path"
                break
            fi
        done
        
        if [ -n "$JUCE_VST3_HELPER" ]; then
            echo -e "${GREEN}Found juce_vst3_helper at: $JUCE_VST3_HELPER${NC}"
            echo -e "${YELLOW}Creating symlink to juce_vst3_helper in /usr/local/bin${NC}"
            sudo ln -sf "$JUCE_VST3_HELPER" /usr/local/bin/juce_vst3_helper
        else
            echo -e "${RED}Could not find juce_vst3_helper. Plugin build may fail.${NC}"
            echo -e "${YELLOW}Building standalone only...${NC}"
            cmake --build build --target MidiPortalStandalone
            BUILD_PLUGIN=false
        fi
    fi
    
    if [ "$BUILD_PLUGIN" = true ]; then
        cmake --build build
    fi
else
    # Build standalone only
    echo -e "${YELLOW}Building standalone only...${NC}"
    cmake --build build --target MidiPortalStandalone
fi

if [ $? -eq 0 ]; then
    echo -e "${GREEN}Build completed successfully!${NC}"
    if [ "$BUILD_TYPE" = "Debug" ]; then
        echo -e "${YELLOW}Run with: ./build/standalone/MidiPortalStandalone${NC}"
    else
        echo -e "${YELLOW}Run with: ./build/standalone/MidiPortalStandalone${NC}"
    fi
else
    echo -e "${RED}Build failed!${NC}"
    exit 1
fi

# Make the script executable
chmod +x build.sh 