#include <iostream>
#include "Surface.h"

Surface::Surface(const char* filePath)
    : Surface::Surface() {
    setSurface(filePath);
}

void Surface::setSurface(const char* filePath) {
    //We load the surface via cairo, and specify the custom cairo deleter
    //std::shared_ptr handles reference counting will automaticly 
    //cleans-up the image data when there are no more references
    mSurface = std::shared_ptr<cairo_surface_t>(cairo_image_surface_create_from_png(filePath), cairo_surface_destroy);
    switch(cairo_surface_status(mSurface.get())) {
        case CAIRO_STATUS_SUCCESS:
            return;
        case CAIRO_STATUS_NULL_POINTER:
            std::cerr << "Oops we messed up on our end." << std::endl << 
                    "Please notify Vaughn that Harry screwed up." << std::endl;
            break;
        case CAIRO_STATUS_NO_MEMORY:
            std::cerr << "Out of memory" << std::endl;
            break;
        case CAIRO_STATUS_READ_ERROR:
            std::cerr << "Error reading file" << std::endl;
            break;
        case CAIRO_STATUS_INVALID_CONTENT:
            std::cerr << "Invalid Content" << std::endl;
            break;
        case CAIRO_STATUS_INVALID_FORMAT:
            std::cerr << "Invalid Format" << std::endl;
            break;
        case CAIRO_STATUS_INVALID_VISUAL:
            std::cerr << "Invalid Visual" << std::endl;
            break;
        default:
            std::cerr << "Invalid Error" << std::endl;
            break;
    }
    // should only reach this point if unsuccessful
    mSurface = std::shared_ptr<cairo_surface_t>(nullptr);
}

cairo_surface_t* Surface::getSurface() const {
    return mSurface.get();
}
