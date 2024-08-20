#include <algorithm>
#include <cctype>
#include <condition_variable>
#include <iostream>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

std::string buffer;
std::mutex mtx;
std::condition_variable cv;
bool ready = false;

bool isAllDigits(const std::string & str) {
    return std::all_of(str.begin(), str.end(), ::isdigit);
}

void processInput() {
    while (true) {
        std::cout << std::endl;
        std::cout << "Thread : " << std::this_thread::get_id() << std::endl;
        std::string input;
        std::cout << "Enter a string of digits (up to 64 characters): ";
        std::cin >> input;

        if (input.length() > 64) {
            std::cout << "Input exceeds 64 characters, try again.\n";
            continue;
        }

        if (!isAllDigits(input)) {
            std::cout << "Input contains non-digit characters, try again.\n";
            continue;
        }

        std::sort(input.begin(), input.end(), std::greater<char>());

        std::string result;
        result.reserve(input.size() * 2);

        for (char ch: input) {
            if ((ch - '0') % 2 == 0) {
                result += "KB";
            } else {
                result += ch;
            }
        }

        std::unique_lock<std::mutex> lock(mtx);
        buffer = result;
        ready  = true;
        cv.notify_one();
        cv.wait(lock, [] { return !ready; });
    }
}

void processData() {
    while (true) {
        std::unique_lock<std::mutex> lock(mtx);
        cv.wait(lock, [] { return ready; });
        std::cout << std::endl;
        std::cout << "Thread : " << std::this_thread::get_id() << std::endl;

        std::cout << "Processed string: " << buffer << std::endl;

        int sum = 0;
        for (char ch: buffer) {
            if (isdigit(ch)) {
                sum += ch - '0';
            }
        }

        std::cout << "Sum of digits: " << sum << std::endl;

        buffer.clear();
        ready = false;
        cv.notify_one();
    }
}

int main() {
    std::thread t1(processInput);
    std::thread t2(processData);

    t1.join();
    t2.join();

    return 0;
}
