#include <errno.h>
#include <fcntl.h>
#include <linux/videodev2.h>
#include <sys/ioctl.h>
#include <unistd.h>

#include <cstring>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>

class CameraException : public std::runtime_error {
public:
    explicit CameraException(const std::string& message)
        : std::runtime_error(message) {}
};

static std::string makeErrorDetail(const std::string& operation,
                                  const std::string& device,
                                  int err = errno) {
    std::ostringstream oss;
    oss << operation << " em " << device
        << " falhou (errno=" << err << ": " << std::strerror(err) << ")";
    return oss.str();
}

static std::string findFirstCameraDevice() {
    for (int i = 0; i < 10; ++i) {
        const std::string device = "/dev/video" + std::to_string(i);
        if (access(device.c_str(), R_OK | W_OK) == 0) {
            return device;
        }
    }
    throw CameraException("Nenhuma câmera foi encontrada em /dev/video*. Verifique se a webcam está conectada e o driver está carregado.");
}

static void tryUseCamera(const std::string& device) {
    int fd = open(device.c_str(), O_RDWR | O_NONBLOCK);
    if (fd < 0) {
        throw CameraException(makeErrorDetail("open()", device));
    }

    try {
        v4l2_capability caps{};
        if (ioctl(fd, VIDIOC_QUERYCAP, &caps) != 0) {
            throw CameraException(makeErrorDetail("VIDIOC_QUERYCAP", device));
        }

        std::cout << "Câmera detectada: " << caps.card << " em " << device << std::endl;

        v4l2_format fmt{};
        fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        fmt.fmt.pix.width = 640;
        fmt.fmt.pix.height = 480;
        fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_YUYV;
        fmt.fmt.pix.field = V4L2_FIELD_NONE;

        if (ioctl(fd, VIDIOC_S_FMT, &fmt) != 0) {
            throw CameraException(makeErrorDetail("VIDIOC_S_FMT (640x480 YUYV)", device));
        }

        v4l2_format actual{};
        actual.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        if (ioctl(fd, VIDIOC_G_FMT, &actual) != 0) {
            throw CameraException(makeErrorDetail("VIDIOC_G_FMT", device));
        }

        std::cout << "Formato aceito pela câmera: "
                  << actual.fmt.pix.width << "x" << actual.fmt.pix.height
                  << " pixelformat=" << actual.fmt.pix.pixelformat << std::endl;

        std::cout << "Sucesso: a câmera foi aberta e configurada em " << device << std::endl;
    } catch (...) {
        close(fd);
        throw;
    }

    close(fd);
}

int main() {
    try {
        const std::string device = findFirstCameraDevice();
        tryUseCamera(device);
        return 0;
    } catch (const std::exception& ex) {
        std::cerr << "EXCEPTION: " << ex.what() << std::endl;
        return 1;
    }
}
