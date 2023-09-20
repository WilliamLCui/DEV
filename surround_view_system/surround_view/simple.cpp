#include <opencv2/opencv.hpp>
#include <vector>

class PointSelector {
public:
    PointSelector(cv::Mat image, std::string title = "PointSelector")
            : image(image), title(title) {}

    std::vector<cv::Point2f> getSelectedPoints() const {
        return keypoints;
    }

    void loop() {
        cv::namedWindow(title);
        cv::setMouseCallback(title, onMouse, this);

        while (true) {
            cv::imshow(title, image);
            int key = cv::waitKey(1) & 0xFF;

            if (key == 'q') {  // Press 'q' to quit
                break;
            } else if (key == 13) {  // Press 'Enter' to confirm
                break;
            } else if (key == 'd') {  // Press 'd' to delete the last point
                if (!keypoints.empty()) {
                    keypoints.pop_back();
                }
            }
        }

        cv::destroyAllWindows();
    }

private:
    static void onMouse(int event, int x, int y, int flags, void* userdata) {
        PointSelector* selector = static_cast<PointSelector*>(userdata);

        if (event == cv::EVENT_LBUTTONDOWN) {
            selector->keypoints.push_back(cv::Point2f(x, y));
        }
    }

    cv::Mat image;
    std::string title;
    std::vector<cv::Point2f> keypoints;
};

int main() {
    cv::Mat image = cv::imread("path/to/image.jpg");

    if (image.empty()) {
        std::cerr << "Failed to load image." << std::endl;
        return 1;
    }

    PointSelector selector(image);
    selector.loop();

    std::vector<cv::Point2f> selectedPoints = selector.getSelectedPoints();

    std::cout << "Selected points:" << std::endl;
    for (const auto& point : selectedPoints) {
        std::cout << "(" << point.x << ", " << point.y << ")" << std::endl;
    }

    return 0;
}

void onclick(int event, int x, int y, int flags, void* param) {
    MyClass* self = static_cast<MyClass*>(param);

    if (event == cv2::EVENT_LBUTTONDOWN) {
        std::cout << "click (" << x << ", " << y << ")" << std::endl;
        self->keypoints.push_back(std::make_pair(x, y));
        self->draw_image();
    }
}

bool loop() {
    cv2::namedWindow(title);
    cv2::setMouseCallback(title, onclick, &self);
    cv2::imshow(title, image);

    while (true) {
        int click = cv2::getWindowProperty(title, cv2::WND_PROP_AUTOSIZE);
        if (click < 0) {
            return false;
        }

        int key = cv2::waitKey(1) & 0xFF;

        // press q to return false
        if (key == 'q') {
            return false;
        }

        // press d to delete the last point
        if (key == 'd') {
            if (self.keypoints.size() > 0) {
                std::pair<int, int> point = self.keypoints.back();
                self.keypoints.pop_back();
                std::cout << "Delete (" << point.first << ", " << point.second << ")" << std::endl;
                self.draw_image();
            }
        }

        // press Enter to confirm
        if (key == 13) {
            return true;
        }
    }
}

cv::Mat create_mask_from_pixels(std::vector<std::pair<int, int>>& pixels, cv::Size image_shape) {
    std::vector<cv::Point> points;
    for (const auto& pixel : pixels) {
        points.push_back(cv::Point(pixel.first, pixel.second));
    }

    cv::Mat mask = cv::Mat::zeros(image_shape, CV_8UC1);
    cv::drawContours(mask, std::vector<std::vector<cv::Point>>{points}, 0, cv::Scalar(255), cv::FILLED);

    return mask;
}

cv::Mat draw_mask_on_image(cv::Mat image, cv::Mat mask) {
    cv::Mat new_image = cv::Mat::zeros(image.size(), image.type());
    new_image.setTo(FILL_COLOR);
    image.copyTo(new_image, mask);
    return new_image;
}

