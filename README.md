# SimpleImageProcessing
Some simple image processing algorithms implemented in c++ using opencv.


In order to build the project:
    $ mkdir build; cd build
    $ cmake ..
    $ make -j
    
In order to run the application:
    $ ./Main ${PATH_IMAGE}
    
 where PATH_IMAGE is the path to the input image. The input image should be in RGB colorspace and was tested with .jpeg images. The processed images will be saved in the same directory as the input image in .jpeg format.
 
When running the application, windows of the image / processed image will appear. In order to advance press any key. The images will also be saved to disk.
 
 The application implements the following image processing algorihms / filters:
 
    - Negative,
    - Grayscale,
    - Binary / black and white,
    - RGB to HSV transformation,
    - Grayscale value histogram,
    - Gray reduction filter,
    - FloydSteinberg algorith.
