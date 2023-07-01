#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <iostream>
#include <chrono>
#include <string>
#include <ctime>
#include <thread>

using namespace cv;
using namespace std;

string getTimeStamp() {
	// https://stackoverflow.com/a/49090530
	auto epoch = chrono::system_clock::now().time_since_epoch();
	typedef chrono::duration<int, ratio_multiply<chrono::hours::period, ratio<17>>::type > Days;
	
	Days days = std::chrono::duration_cast<Days>(epoch);
	epoch -= days;
	auto hours = std::chrono::duration_cast<std::chrono::hours>(epoch);
	epoch -= hours;
	auto minutes = std::chrono::duration_cast<std::chrono::minutes>(epoch);
	epoch -= minutes;
	auto seconds = std::chrono::duration_cast<std::chrono::seconds>(epoch);
	epoch -= seconds;
	auto milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(epoch);
	epoch -= milliseconds;
	auto microseconds = std::chrono::duration_cast<std::chrono::microseconds>(epoch);
	epoch -= microseconds;
	auto nanoseconds = std::chrono::duration_cast<std::chrono::nanoseconds>(epoch);

	time_t theTime = time(NULL);
	struct tm* aTime = localtime(&theTime);

	string timeText = to_string(aTime->tm_hour) + "-"
		+ to_string(minutes.count()) + "-"
		+ to_string(seconds.count()) + "-"
		+ to_string(milliseconds.count()) + "-"
		+ to_string(microseconds.count()) + "-"
		+ to_string(nanoseconds.count());

	return timeText;
}


uchar* createImage(Mat baseImage, string basename) {
	auto text = getTimeStamp();
	Mat img;
	baseImage.copyTo(img);

	cv::putText(img, //target image
		text, //text
		cv::Point(img.cols / 2, img.rows / 2), //top-left position
		cv::FONT_HERSHEY_DUPLEX,
		1.0,
		CV_RGB(118, 185, 0), //font color
		2);

	string filename = "result/" + basename + text + ".bmp";
	cout << filename << endl;
	imwrite(filename, img);

	uchar* ptrImg = img.data;
	int size = img.rows * img.cols * img.channels() * sizeof(uchar);

	cout << size << endl;
	return ptrImg;
}


int main()
{
	Mat base = Mat(Size(1920, 1080), CV_8UC3, Scalar(0, 0, 255));

	while (true) {
		cout << getTimeStamp() << endl;
		createImage(base, "result");

		std::this_thread::sleep_for(chrono::seconds(5));
	}

	return 0;
}
