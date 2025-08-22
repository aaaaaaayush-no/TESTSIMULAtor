#include "Constants.h"
#include <iostream>
#include <vector>
#include <string>

// Load textures for all gates
void LoadGateTextures() {
    std::cout << "\n===== TEXTURE LOADING DIAGNOSTIC =====\n";
    std::cout << "Current working directory: " << GetWorkingDirectory() << std::endl;
    
    // Try multiple potential resource paths
    std::vector<std::string> possiblePaths = {
        "",                // Current directory
        "resources/",      // Resources subfolder
        "../resources/",   // Parent directory's resources folder
        "../../resources/", // Grandparent directory's resources folder
        "./resources/"     // Explicit current directory
    };
    
    // GATE_DATA is now properly mutable - no const_cast needed
    bool anyTextureLoaded = false;
    
    for (auto& pair : GATE_DATA) {
        GateInfo& info = pair.second;
        
        if (info.imagePath == nullptr) {
            std::cout << "- No image path for " << info.label << " (this is expected for INPUT/OUTPUT)" << std::endl;
            continue;
        }
        
        bool textureLoaded = false;
        const char* filename = info.imagePath;
        
        // Extract just the filename part (remove "resources/" if present)
        const char* actualFilename = filename;
        if (strstr(filename, "/") != nullptr) {
            actualFilename = strrchr(filename, '/') + 1;
        }
        
        // Try each possible path
        for (const auto& basePath : possiblePaths) {
            std::string fullPath = basePath + actualFilename;
            
            if (FileExists(fullPath.c_str())) {
                info.texture = LoadTexture(fullPath.c_str());
                
                if (info.texture.id != 0) {
                    // Apply texture smoothing/filtering
                    SetTextureFilter(info.texture, TEXTURE_FILTER_BILINEAR);
                    
                    std::cout << "✓ Successfully loaded " << info.label << " texture from: " << fullPath << std::endl;
                    std::cout << "  ID: " << info.texture.id << ", Size: " 
                             << info.texture.width << "x" << info.texture.height << " (Smoothed)" << std::endl;
                    textureLoaded = true;
                    anyTextureLoaded = true;
                    break;
                } else {
                    std::cout << "⚠ Found but failed to load: " << fullPath << std::endl;
                }
            } else {
                std::cout << "- Tried path: " << fullPath << " (file not found)" << std::endl;
            }
        }
        
        if (!textureLoaded) {
            std::cout << "✗ ERROR: Could not load texture for " << info.label << std::endl;
        }
    }
    
    if (!anyTextureLoaded) {
        std::cout << "\n⚠ CRITICAL: No textures were loaded! Please check:\n";
        std::cout << "  1. Image files exist in a 'resources' directory\n";
        std::cout << "  2. Images are in PNG format\n";
        std::cout << "  3. Images have appropriate read permissions\n";
        std::cout << "  4. Try placing images in the same directory as the executable\n";
    }
    
    std::cout << "======================================\n\n";
}

// Unload all gate textures
void UnloadGateTextures() {
    for (auto& pair : GATE_DATA) {
        if (pair.second.texture.id != 0) {
            std::cout << "Unloading texture: " << pair.second.label << " (ID: " << pair.second.texture.id << ")" << std::endl;
            UnloadTexture(pair.second.texture);
            pair.second.texture.id = 0;
        }
    }
}