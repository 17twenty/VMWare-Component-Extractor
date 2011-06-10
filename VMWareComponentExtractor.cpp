// This is free software developed by Nick Glynn (n.s.glynn@gmail.com)
// to extract VMWare components from the component.tar files.
// If you make modifications to this code, please feed them back to me so I
// can improve this for everyone else.
//
// This software is nothing to do with VMWare

/// \TODO: Make this extract all of the files (I'm sure they have a use 
/// somewhere) rather than just the ISO but that was all I needed.

#include <fstream>
#include <string>
#include <cstdlib>
#include <iostream>

const std::string TAR_EXTRACT_COMMAND    = "tar -xf ";
const std::string GUNZIP_COMMAND         = "gunzip ";
const std::string GZIP_EXT		 = ".gz";


/**
 * Checks existence of a file
 * file is closed when it goes out of scope
 */
bool FileExists(const std::string &fileName) {
    std::ifstream fileToCheck(fileName.c_str());
    return fileToCheck;
}

/**
  * Returns filename of file sans extension
  */
std::string StripExtension(const std::string &fileName) {
    return fileName.substr(0, fileName.find_last_of('.'));
}

/**
  * Extract single object between offset->length and stores
  * as outputFile
  */
bool ExtractObjectFromStream(std::ifstream &input, std::string outputFileName, 
    const long &offset, const long &length) {
    bool success = false;
    outputFileName += GZIP_EXT;
    std::ofstream outputFile(outputFileName.c_str(), std::ios::binary);
    
    if (outputFile) {
        input.seekg(offset, std::ios::beg);
        
        long dataRemaining = length;
        while (dataRemaining--) {
            outputFile.put(input.get());
        }
        outputFile.close();
        std::string execCommand = GUNZIP_COMMAND + outputFileName;
        success = (0 == system(execCommand.c_str()));
    }
    
    return success;
}

/**
  * Churns through the XML tag given extracting required values
  * libXML is waaaay too heavy for this task
  */
std::string GetValueFromTag(std::string source, std::string tag) {
    // We cannibalise source & tag so don't bother with PBR
    tag += "=\"";
    std::size_t found = source.find(tag);
    std::string value = source.substr(found);
    found = value.find("\"");
    value = value.substr(found + 1);
    std::size_t end = value.find("\"");
    value = value.substr(0, end);

    return value;
}

/**
  * Reads through XML header for offset/sizes/files and extracts them
  * \\\ /TODO: Expand this to read all files from archive
  */
bool ExtractObjectsFromComponent(const std::string &componentFileName) {
    bool success = false;
    
    std::ifstream component(componentFileName.c_str(), std::ios::in | std::ios::binary);
    if (component) {
        std::string tag;
        long dataOffset = 0;
        long isoOffset = 0;
        long isoLength = 0;
        std::string isoName = "";
        while (!component.eof() && (0l == dataOffset)) {
            tag = "";
            char c = component.get();
            if ('<' == c) {
                while(c != '>') {
                    c = component.get();
                    if (c != '>')
                        tag+= c;
                
                }
                // Do something with tag
                if (std::string::npos != tag.find("/component")) {
                    dataOffset = component.tellg();
                } else if (std::string::npos != tag.find("file path")) {
                    // Extract the compressed size, length and name
                    if (std::string::npos != tag.find(".iso\"")) {
                        isoName = GetValueFromTag(tag, "file path");
                        isoLength = strtol(GetValueFromTag(tag, "compressedSize").c_str(), NULL, 0);
                        isoOffset = strtol(GetValueFromTag(tag, "offset").c_str(), NULL, 0);
                    }
                }
                
                
                tag = "";
            }
        }
        success = ExtractObjectFromStream(component, isoName, isoOffset + dataOffset, isoLength);
    }
    
    return success;
}

int main(int argumentCount, char **arguments) {
    if (2 == argumentCount && FileExists(arguments[1])) {
        std::string fileToExtract = StripExtension(arguments[1]);
        std::string tarCommand = TAR_EXTRACT_COMMAND \
            + std::string(arguments[1]) \
            + " " + fileToExtract;
        
        std::system(tarCommand.c_str());
        if (!FileExists(fileToExtract)) {
            std::cout << "Failed to extract component file" << std::endl;
            goto error;
        }
        
        if (!ExtractObjectsFromComponent(fileToExtract)) {
            std::cout << "Failed to extract all files from component file" << std::endl;
            goto error;
        }
    } else {
        std::cout << "Usage:" << std::endl <<  std::string(arguments[0])
	    << " <componentfilename.component.tar>" << std::endl;
    }

    // Operation completed fall out happy. Normally dislike multiple return points but
    // this works well enough for what we need.
    return 0;
    
error:
    return -1;    
}
