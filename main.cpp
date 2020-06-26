#include <opencv4/opencv2/opencv.hpp>
#include <string>
#include <vector>


#define IMAGE_FORMAT ".jpeg"
#define max(a, b, c) ((a) > (b)) ? ((a) > (c) ? (a) : (c)) : ((b) > (c) ? (b) : (c))
#define min(a, b, c) ((a) < (b)) ? ((a) < (c) ? (a) : (c)) : ((b) < (c) ? (b) : (c))


cv::Mat negateImage(cv::Mat);
cv::Mat greyscaleImage(cv::Mat);
cv::Mat binaryScaleImage(cv::Mat, float);
cv::Mat RGBtoHSVImage(cv::Mat);
unsigned int* get8UC1ImageHistogram(cv::Mat);
std::vector<unsigned char> getHistMaximums(float*, int, float);
unsigned char getPixelMax(std::vector<unsigned char>, unsigned char);
cv::Mat grayReduction(std::vector<unsigned char>, cv::Mat);
cv::Mat FloydSteinbergAlg(cv::Mat, std::vector<unsigned char>);
void showHistogram(const std::string name, unsigned int* hist, const int hist_cols, const int hist_height);

int main (int argc, char *argv[]) {

	if (argc < 2) {
		std::cout << "Missing path to image!" << std::endl;
		exit(1);
	}

	std::string imageFormat = IMAGE_FORMAT;
	std::string pathToImage = argv[1];
	std::string pathToWrite = pathToImage.substr(0, pathToImage.size() - imageFormat.size());

	cv::Mat image = cv::imread(pathToImage, cv::IMREAD_UNCHANGED);


	// Show base image.
	std::cout << "Showing base image...\n" << std::endl;

	cv::namedWindow(pathToImage, cv::WINDOW_AUTOSIZE);

	cv::imshow(pathToImage, image);
	cv::waitKey(0);


	// Negative filter.
	std::cout << "Showing negative filter...\n" << std::endl;

	cv::Mat negativeImage = negateImage(image);

	cv::imwrite(pathToWrite + "Negative" + IMAGE_FORMAT, negativeImage);
	cv::imshow(pathToImage, negativeImage);
	cv::waitKey(0);


	// Greyscale filter.
	std::cout << "Showing greyscale filter...\n" << std::endl;

	cv::Mat greyscaledImage = greyscaleImage(image);

	cv::imwrite(pathToWrite + "Greyscale" + IMAGE_FORMAT, greyscaledImage);
	cv::imshow(pathToImage, greyscaledImage);
	cv::waitKey(0);


	// Black and white filter.
	std::cout << "Showing black and white filter...\n" << std::endl;

	cv::Mat blackAndWhiteImage = binaryScaleImage(greyscaledImage, 0.25);
	cv::imwrite(pathToWrite + "BlackAndWhite" + IMAGE_FORMAT, blackAndWhiteImage);
	cv::imshow(pathToImage, blackAndWhiteImage);
	cv::waitKey(0);


	// RGB to HSV transformation.
	std::cout << "Showing RGB -> HSV transformation...\n" << std::endl;

	cv::Mat HSVImage = RGBtoHSVImage(image);
	cv::imwrite(pathToWrite + "HSVImage" + IMAGE_FORMAT, HSVImage);
	cv::imshow(pathToImage, HSVImage);
	cv::waitKey(0);


	// Image histogram.
	std::cout << "Showing histogram...\n" << std::endl;

	unsigned int *histogram = get8UC1ImageHistogram(greyscaledImage);
	unsigned int numberOfPixels = 0;

//	std::cout << "The histogram of the image (pixel_value: number_of_pixels): " << std::endl;
	for (int i = 0; i < 256; i++) {

//		std::cout << i << ": " << (int)histogram[i] << std::endl;

		numberOfPixels += (int)histogram[i];
	}

	if (numberOfPixels == (greyscaledImage.rows * greyscaledImage.cols)){
//		std::cout << "Analyzed " << numberOfPixels << " out of " << numberOfPixels << std::endl;
	} else {
		std::cout << "Error analyzing pixels!";
	}

	showHistogram(pathToImage, histogram, 256, 100);

	// Calculate function of distributed probability.
	float* FDP = new float[256]();
	float check = 0;

//	std::cout << "FDP\n" << std::endl;
	for (int i = 0; i < 256; i++) {
		FDP[i] = (float)histogram[i] / numberOfPixels;
		check += FDP[i];
//		std::cout << "FDP[" << i << "]= " << FDP[i] << std::endl;
	}

	if (check != 1.0f) {
		std::cout << "Error while calculating FDP! " << check << std::endl;
	}


	// Histogram maximums.
	std::vector<unsigned char> maximums = getHistMaximums(FDP, 5, 0.0003);

	// Gray Reduction filter.
	std::cout << "Showing grey reduction filter...\n" << std::endl;

	cv::Mat grayReducedImage = grayReduction(maximums, greyscaledImage);
	cv::imwrite(pathToWrite + "GrayscaleReducedImage" + IMAGE_FORMAT, grayReducedImage);
	cv::imshow(pathToImage, grayReducedImage);
	cv::waitKey(0);


	// Floyd-Steinberg algorithm.
	std::cout << "Showing FloydSteinbergAlg...\n" << std::endl;

	cv::Mat FloydSteinbergImage = FloydSteinbergAlg(greyscaleImage(image), maximums);
	cv::imwrite(pathToWrite + "Floyd" + IMAGE_FORMAT, FloydSteinbergImage);
	cv::imshow("ASDF", FloydSteinbergImage);
	cv::waitKey(0);

	return 0;
} 


cv::Mat negateImage(cv::Mat baseImage) {

	cv::Mat image = baseImage.clone();

	for (int i = 0; i < image.rows; i++) {
		for (int j = 0; j < image.cols; j++) {

			cv::Vec3b pixel = image.at<cv::Vec3b>(i, j);
			pixel[0] = 255 - pixel[0];
			pixel[1] = 255 - pixel[1];
			pixel[2] = 255 - pixel[2];

			image.at<cv::Vec3b>(i, j) = pixel;
		}
	}

	return image;
}

cv::Mat greyscaleImage(cv::Mat baseImage) {

	cv::Mat buffImage = baseImage.clone();
	cv::Mat image = cv::Mat(baseImage.rows, baseImage.cols, CV_8UC1, cv::Scalar::all(0));

	for (int i = 0; i < image.rows; i++) {
		for (int j = 0; j < image.cols; j++) {

			cv::Vec3b pixel = buffImage.at<cv::Vec3b>(i, j);

			image.at<unsigned char>(i, j) = (unsigned char)((pixel[0] + pixel[1] + pixel[2]) / 3);
		}
	}
	return image;
}

cv::Mat binaryScaleImage(cv::Mat baseImage, float threshold) {

	cv::Mat image = baseImage.clone();

	threshold = threshold * 255;

	for (int i = 0; i < image.rows; i++) {
		for (int j = 0; j < image.cols; j++) {

			if (image.at<unsigned char>(i, j) < (unsigned char)threshold) {
				image.at<unsigned char>(i, j) = 0;
			} else {
				image.at<unsigned char>(i, j) = 255;
			}
		}
	}

	return image;
}

cv::Mat RGBtoHSVImage(cv::Mat baseImage) {

	cv::Mat image = baseImage.clone();

	float H, S, V, r, g, b;

	for (int i = 0; i < image.rows; i++) {
		for (int j = 0; j < image.cols; j++) {

			cv::Vec3b pixel = image.at<cv::Vec3b>(i, j);

			// Normalize inputs.
			b = (float)pixel[0] / 255;
			g = (float)pixel[1] / 255;
			r = (float)pixel[2] / 255;

			float M = max(r, g, b);
			float m = min(r, g, b);
			float C = M - m;
//			std::cout << "r= " << r << "\ng= " << g << "\nb= " << b << "\nMax= " << M << "\nMin= " << m << std::endl;

			// Value.
			V = M;

			// Saturation.
			if (C) {
				S = C / V;
			} else {
				S = 0;
			}

			// Hue.
			if (C) {
				if (M == r) {
					H = 60 * (g - b) / C;
				}
				if (M == g) {
					H = 120 + 60 * (b - r) / C;
				}
				if (M == b) {
					H = 240 + 60 * (r - g) / C;
				}
			} else {
				H = 0;
			}

			if (H < 0) {
				H = H + 360;
			}

			H = H * 255 / 360;
			S = S * 255;
			V = V * 255;

			image.at<cv::Vec3b>(i, j) = cv::Vec3b((unsigned char)H, (unsigned char)S, (unsigned char)V);
		}
	}

	return image;
}

unsigned int* get8UC1ImageHistogram(cv::Mat baseImage) {

	cv::Mat image = baseImage.clone();

	unsigned int* histogram = new unsigned int[256]();

	for (int i = 0; i < image.rows; i++) {
		for (int j = 0; j < image.cols; j++) {

			histogram[image.at<unsigned char>(i, j)] ++;
		}
	}

	return histogram;
}

std::vector<unsigned char> getHistMaximums(float* FDP, int wh, float TH){

	std::vector<unsigned char> maximums;
	maximums.push_back(0);
	int currentBin = 1;

	for (int k = wh; k < 255 - wh; k++) {

		float mean = 0;
		int condition = 1;

		for (int i = (k - wh); i < (k + wh); i++) {
			mean += FDP[i];
			if (FDP[k] < FDP[i]) {
				condition = 0;
			}
		}

		mean /= 2*wh+1;

		if (FDP[k] > (mean + TH) && condition) {

//			std::cout << "curr: " << currentBin << " k: " << k << std::endl;
			maximums.push_back(k);
		}
	}
	maximums.push_back(255);
//	std::cout << "Bins: " << currentBin << " +2" << std::endl;

	return maximums;
}

unsigned char getPixelMax(std::vector<unsigned char> maximums, unsigned char pixel) {

	if (pixel == 0 || pixel == 255) {
		return pixel;
	}

	for (int i = 1; i < maximums.size()-1; i++) {
		if (pixel >= maximums[i] && pixel < maximums[i+1]) {
			return maximums[i];
		}
	}
}

cv::Mat grayReduction(std::vector<unsigned char> maximums, cv::Mat baseImage) {

	cv::Mat image = baseImage.clone();

	for (int i = 0; i < image.rows; i++) {
		for (int j = 0; j < image.cols; j++) {

			unsigned char pixel = image.at<unsigned char>(i, j);
			image.at<unsigned char>(i, j) = getPixelMax(maximums, pixel);
		}
	}

	return image;
}

cv::Mat FloydSteinbergAlg(cv::Mat baseImage, std::vector<unsigned char> maximums) {

	cv::Mat image = baseImage.clone();

	for (int i = 0; i < image.rows; i++) {
		for (int j = 0; j < image.cols; j++) {

			unsigned char oldPixel = image.at<unsigned char>(i, j);
			unsigned char newPixel = getPixelMax(maximums, oldPixel);
			image.at<unsigned char>(i, j) = newPixel;

			char error = oldPixel - newPixel;
			if ((i+1) < image.rows-1){
				//std::cout << "!\n";
				unsigned char add = image.at<unsigned char>(i+1, j) + ((7*error)/16);
				image.at<unsigned char>(i+1, j  ) = add;
			}
			if ((i-1) >= 0 && (j+1) < image.cols-1) {
				//std::cout << "!!\n";
				unsigned char add = image.at<unsigned char>(i-1, j+1) + ((3*error)/16);
				image.at<unsigned char>(i-1, j+1) = add;
			}
			if ((j+1) < image.cols-1){
				//std::cout << "!!!\n";
				unsigned char add = image.at<unsigned char>(i  , j+1) + ((5*error)/16);
				image.at<unsigned char>(i  , j+1) = add;
			}
			if (((j+1) < image.cols-1) && ((i+1) <= image.rows-1)){
				//std::cout << "!!!!\n";
				unsigned char add = image.at<unsigned char>(i+1, j+1) + (error/16);
				image.at<unsigned char>(i+1, j+1) = add;
			}
		} 
	}

	return image;
}

void showHistogram(const std::string name, unsigned int* hist, const int hist_cols, const int hist_height) {

	cv::Mat imgHist(hist_height, hist_cols, CV_8UC3, CV_RGB(255, 255, 255)); // constructs a white image
	//computes histogram maximum
	int max_hist = 0;
	for (int i = 0; i<hist_cols; i++)
		if (hist[i] > max_hist)
			max_hist = hist[i];

	double scale = 1.0;
	scale = (double)hist_height / max_hist;
	int baseline = hist_height - 1;
	for (int x = 0; x < hist_cols; x++) {
		cv::Point p1 = cv::Point(x, baseline);
		cv::Point p2 = cv::Point(x, baseline - cvRound(hist[x] * scale));
		line(imgHist, p1, p2, CV_RGB(255, 0, 255)); // histogram bins // colored in magenta
	}
	cv::imshow(name, imgHist);
	cv::waitKey(0);

}