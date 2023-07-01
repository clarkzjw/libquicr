
#include <chrono>
#include <cstring>
#include <iostream>
#include <quicr/quicr_client.h>
#include <quicr/quicr_common.h>
#include <sstream>
#include <thread>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <thread>
#include <fstream>

// #include <opencv2/core.hpp>
// #include <opencv2/imgcodecs.hpp>
// #include <opencv2/highgui.hpp>
// #include <opencv2/imgproc.hpp>
// using namespace cv;

#include <iostream>
#include <chrono>
#include <string>
#include <ctime>
#include <thread>

#include "testLogger.h"

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


// int createImage(Mat& baseImage, string basename, int &size) {
// 	auto text = getTimeStamp();
// 	// Mat img;
// 	// baseImage.copyTo(img);

// 	cv::putText(baseImage, //target image
// 		text, //text
// 		cv::Point(baseImage.cols / 2, baseImage.rows / 2), //top-left position
// 		cv::FONT_HERSHEY_DUPLEX,
// 		1.0,
// 		CV_RGB(118, 185, 0), //font color
// 		2);

// 	string filename = "result/" + basename + text + ".bmp";
// 	cout << filename << endl;
// 	imwrite(filename, baseImage);

// 	// uchar* ptrImg = img.data;
// 	// int size = img.rows * img.cols * img.channels() * sizeof(uchar);
//   size = baseImage.total() * baseImage.elemSize();

// 	// cout << size << endl;
// 	// return ptrImg;
//   return 0;
// }


class subDelegate : public quicr::SubscriberDelegate
{
public:
  subDelegate(testLogger& logger)
    : logger(logger)
  {
  }

  void onSubscribeResponse(
    [[maybe_unused]] const quicr::Namespace& quicr_namespace,
    [[maybe_unused]] const quicr::SubscribeResult& result) override
  {

    std::stringstream log_msg;
    log_msg << "onSubscriptionResponse: name: " << quicr_namespace << "/"
            << int(quicr_namespace.length())
            << " status: " << int(static_cast<uint8_t>(result.status));

    logger.log(qtransport::LogLevel::info, log_msg.str());
  }

  void onSubscriptionEnded(
    [[maybe_unused]] const quicr::Namespace& quicr_namespace,
    [[maybe_unused]] const quicr::SubscribeResult::SubscribeStatus& reason)
    override
  {

    std::stringstream log_msg;
    log_msg << "onSubscriptionEnded: name: " << quicr_namespace << "/"
            << int(quicr_namespace.length());

    logger.log(qtransport::LogLevel::info, log_msg.str());
  }

  void onSubscribedObject([[maybe_unused]] const quicr::Name& quicr_name,
                          [[maybe_unused]] uint8_t priority,
                          [[maybe_unused]] uint16_t expiry_age_ms,
                          [[maybe_unused]] bool use_reliable_transport,
                          [[maybe_unused]] quicr::bytes&& data) override
  {
    std::stringstream log_msg;

    log_msg << "recv object: name: " << quicr_name
            << " data sz: " << data.size();

    if (data.size()) {
      log_msg << data.data();

      std::ofstream output("output", std::ios::binary);
      std::cout << sizeof(uint8_t) << std::endl;
      std::cout << data.size() << std::endl;
      output.write(reinterpret_cast<char*>(&data[0]), sizeof(uint8_t) * data.size());
      output.close();
    }

    logger.log(qtransport::LogLevel::info, log_msg.str());
  }

  void onSubscribedObjectFragment(
    [[maybe_unused]] const quicr::Name& quicr_name,
    [[maybe_unused]] uint8_t priority,
    [[maybe_unused]] uint16_t expiry_age_ms,
    [[maybe_unused]] bool use_reliable_transport,
    [[maybe_unused]] const uint64_t& offset,
    [[maybe_unused]] bool is_last_fragment,
    [[maybe_unused]] quicr::bytes&& data) override
  {
  }

private:
  testLogger& logger;
};

class pubDelegate : public quicr::PublisherDelegate
{
public:
  void onPublishIntentResponse(
    [[maybe_unused]] const quicr::Namespace& quicr_namespace,
    [[maybe_unused]] const quicr::PublishIntentResult& result) override
  {
  }
};

int
main(int argc, char* argv[])
{
  auto pd = std::make_shared<pubDelegate>();
  if ((argc != 2) && (argc != 3)) {
    std::cerr
      << "Relay address and port set in RELAY_RELAY and REALLY_PORT env "
         "variables."
      << std::endl;
    std::cerr << std::endl;
    std::cerr << "Usage PUB: reallyTest FF0001 pubData" << std::endl;
    std::cerr << "Usage SUB: reallyTest FF0000" << std::endl;
    exit(-1);
  }

  testLogger logger;

  char* relayName = getenv("REALLY_RELAY");
  if (!relayName) {
    static char defaultRelay[] = "127.0.0.1";
    relayName = defaultRelay;
  }

  int port = 2048;
  char* portVar = getenv("REALLY_PORT");
  if (portVar) {
    port = atoi(portVar);
  }

  auto name = quicr::Name(std::string(argv[1]));

  std::stringstream log_msg;

  log_msg << "Name = " << name;
  logger.log(qtransport::LogLevel::info, log_msg.str());

  std::vector<uint8_t> data;
  if (argc == 3) {
    data.insert(
      data.end(), (uint8_t*)(argv[2]), ((uint8_t*)(argv[2])) + strlen(argv[2]));
  }

  log_msg.str("");
  log_msg << "Connecting to " << relayName << ":" << port;
  logger.log(qtransport::LogLevel::info, log_msg.str());

  quicr::RelayInfo relay{ .hostname = relayName,
                          .port = uint16_t(port),
                          .proto = quicr::RelayInfo::Protocol::QUIC };

  if (data.size() > 0) {
    qtransport::TransportConfig tcfg{ .tls_cert_filename = NULL,
                                  .tls_key_filename = NULL,
                                  .debug = true };
    quicr::QuicRClient client(relay, tcfg, logger);

    auto nspace = quicr::Namespace(name, 96);
    logger.log(qtransport::LogLevel::info,
               "Publish Intent for name: " + std::string(name) +
                 " == namespace: " + std::string(nspace));
    client.publishIntent(pd, nspace, {}, {}, {});

    // do publish
    logger.log(qtransport::LogLevel::info, "Publish");

    //get length of file
#if 1
    std::ifstream infile("input");
    infile.seekg(0, std::ios::end);
    size_t length = infile.tellg();
    infile.seekg(0, std::ios::beg);
    std::cout << length << std::endl;

    std::vector<char> buffers(length);

    infile.read(&buffers[0], length);
    quicr::bytes buf(buffers.begin(), buffers.end());
#endif

#if 0
    Mat base = Mat(Size(1920, 1080), CV_8UC3, Scalar(0, 0, 255));
    uchar *ptrImg = base.data;
    int size = 0;
    createImage(base, "result", size);

    std::cout << "buf size " << size << std::endl;
    quicr::bytes buf(ptrImg, ptrImg + size + 54);

    cout << buf.size() << endl;
#endif
    client.publishNamedObject(name, 0, 10000, false, std::move(buf));

  } else {
    // do subscribe
    qtransport::TransportConfig tcfg{ .tls_cert_filename = NULL,
                                    .tls_key_filename = NULL,
                                    .debug = false };
    quicr::QuicRClient client(relay, tcfg, logger);


    logger.log(qtransport::LogLevel::info, "Subscribe");
    auto sd = std::make_shared<subDelegate>(logger);
    auto nspace = quicr::Namespace(name, 96);

    log_msg.str(std::string());
    log_msg.clear();

    log_msg << "Subscribe to " << name << "/" << 96;
    logger.log(qtransport::LogLevel::info, log_msg.str());

    quicr::SubscribeIntent intent = quicr::SubscribeIntent::immediate;
    quicr::bytes empty;
    client.subscribe(sd, nspace, intent, "origin_url", false, "auth_token", std::move(empty));

    while (true) {
      // logger.log(qtransport::LogLevel::info, "Sleeping");
      std::this_thread::sleep_for(std::chrono::seconds(1));
    }

  }
  std::this_thread::sleep_for(std::chrono::seconds(10));

  return 0;
}
