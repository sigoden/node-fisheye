#include <napi.h>
#include <opencv2/core.hpp>
#include <opencv2/calib3d.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/imgcodecs.hpp>

cv::Mat toImageMat(Napi::Buffer<uchar> jsRawImg, int flag = cv::IMREAD_COLOR)
{
    uchar *buf = jsRawImg.Data();
    size_t size = jsRawImg.Length();
    std::vector<uchar> imgBytes(buf, buf + size);
    cv::Mat img = cv::imdecode(imgBytes, flag);
    return img;
}

cv::Matx33d getK(Napi::Array jsArray)
{
    cv::Mat mat(3, 3, CV_64F);
    uint32_t h = 3, w = 3;
    for (uint32_t i = 0; i < h; i++)
    {
        Napi::Array row = jsArray.Get(i).As<Napi::Array>();
        for (uint32_t j = 0; j < w; j++)
        {
            Napi::Number v = row.Get(j).As<Napi::Number>();
            mat.at<double>(i, j) = v.DoubleValue();
        }
    }
    cv::Matx33d m33((double *)mat.ptr());
    return m33;
}

cv::Vec4d getD(Napi::Array jsArray)
{
    cv::Vec4d mat(
        jsArray.Get(uint32_t(0)).As<Napi::Number>().DoubleValue(),
        jsArray.Get(uint32_t(1)).As<Napi::Number>().DoubleValue(),
        jsArray.Get(uint32_t(2)).As<Napi::Number>().DoubleValue(),
        jsArray.Get(uint32_t(3)).As<Napi::Number>().DoubleValue());
    return mat;
}

Napi::Buffer<char> encodeMat(Napi::Env env, cv::Mat img, Napi::Object jsExtra) {
    cv::String ext;
    std::vector<int> params = std::vector<int>();
    if (jsExtra.Has("extname")) {
        ext = cv::String(jsExtra.Get("extname").As<Napi::String>().Utf8Value());
    } else {
        ext = cv::String(".jpg");
    }
    if (jsExtra.Has("quantity")) {
        int quantity = int(jsExtra.Get("quantity").As<Napi::Number>().Int32Value());
        if (ext == ".jpg" || ext == ".jpeg") {
            params.push_back(cv::IMWRITE_JPEG_QUALITY);
            params.push_back(quantity);
        } else if (ext == ".png") {
            params.push_back(cv::IMWRITE_PNG_COMPRESSION);
            params.push_back(quantity);
        } else if (ext == "webp") {
            params.push_back(cv::IMWRITE_WEBP_QUALITY);
            params.push_back(quantity);
        }
    }
    std::vector<uchar> buf;
    cv::imencode(ext, img, buf, params);

    Napi::Buffer<char> ret = Napi::Buffer<char>::Copy(env, reinterpret_cast<char*>(buf.data()), buf.size());
    return ret;
}

Napi::Buffer<char> Undistort(const Napi::CallbackInfo &info)
{
    Napi::Env env = info.Env();

    Napi::Buffer<uchar> jsRawImg = info[0].As<Napi::Buffer<uchar>>();
    Napi::Array jsK = info[1].As<Napi::Array>();
    Napi::Array jsD = info[2].As<Napi::Array>();
    
    Napi::Object jsExtra;

    if (info.Length() == 4) {
        jsExtra = info[3].As<Napi::Object>();
    } else {
        jsExtra = Napi::Object::New(env);
    }

    cv::Mat distorted = toImageMat(jsRawImg);
    cv::Matx33d k = getK(jsK);
    cv::Vec4d d = getD(jsD);
    cv::Mat undistorted;

    cv::Size size = distorted.size();
    float scale = 1.0;
    if (jsExtra.Has("scale")) {
        scale = jsExtra.Get("scale").As<Napi::Number>().FloatValue();
    }
    size.width *= scale;
    size.height *= scale;

    cv::fisheye::undistortImage(distorted, undistorted, k, d, k, size);

    return encodeMat(env, undistorted, jsExtra);
}

std::vector<cv::Mat> getImages(Napi::Array jsImagesArray)
{
    std::vector<cv::Mat> ret;
    for (uint32_t i = 0; i < jsImagesArray.Length(); i++)
    {
        Napi::Buffer<uchar> img = jsImagesArray.Get(i).As<Napi::Buffer<uchar>>();
        ret.push_back(toImageMat(img, cv::IMREAD_GRAYSCALE));
    }
    return ret;
}

Napi::Array convertK(Napi::Env env, cv::Matx33d theK)
{
    Napi::Array ret = Napi::Array::New(env);
    for (int i = 0; i < theK.rows; i++)
    {
        Napi::Array row = Napi::Array::New(env);
        for (int j = 0; j < theK.cols; j++)
        {
            row.Set(uint32_t(j), Napi::Number::New(env, static_cast<double>(theK(i, j))));
        }
        ret.Set(uint32_t(i), row);
    }
    return ret;
}

Napi::Array convertD(Napi::Env env, cv::Vec4d theD)
{
    Napi::Array ret = Napi::Array::New(env);
    for (unsigned int i = 0; i < sizeof(theD.val) / sizeof(theD.val[0]); i++)
    {
        ret.Set(uint32_t(i), Napi::Number::New(env, static_cast<double>(theD.val[i])));
    }
    return ret;
}

std::vector<cv::Point3f> calibratePattern(cv::Size checkboardSize, float squareSize)
{
    std::vector<cv::Point3f> ret;
    for (int i = 0; i < checkboardSize.height; i++)
    {
        for (int j = 0; j < checkboardSize.width; j++)
        {
            ret.push_back(cv::Point3f(float(j*squareSize), float(i*squareSize), 0));
        }
    }
    return ret;
}

Napi::Object Calibrate(const Napi::CallbackInfo &info)
{
    Napi::Env env = info.Env();

    Napi::Array jsImagesArray = info[0].As<Napi::Array>();
    Napi::Number jsCheckboardWidth = info[1].As<Napi::Number>();
    Napi::Number jsCheckboardHeight = info[2].As<Napi::Number>();

    cv::Size checkboardSize(jsCheckboardWidth.Int32Value(), jsCheckboardHeight.Int32Value());

    std::vector<cv::Mat> images = getImages(jsImagesArray);
    std::vector<std::vector<cv::Point3f> > objPoints;
    std::vector<cv::Mat> imgPoints;

    cv::Matx33d theK;
    cv::Vec4d theD;

    cv::Size imgSize;

    std::vector<cv::Point3f> pattern = calibratePattern(checkboardSize, 1.0);
    cv::TermCriteria subpixCriteria(cv::TermCriteria::EPS | cv::TermCriteria::MAX_ITER, 30, 0.1);
    for (auto const &img : images)
    {
        cv::Mat corners;
        bool found = cv::findChessboardCorners(img, checkboardSize, corners);
 
        if (found)
        {
            objPoints.push_back(pattern);
            cv::cornerSubPix(img, corners, cv::Size(3, 3), cv::Size(-1, -1), subpixCriteria);
            imgPoints.push_back(corners);
        }
    }

    cv::Size size = images.at(0).size();
    int flag = cv::fisheye::CALIB_RECOMPUTE_EXTRINSIC | cv::fisheye::CALIB_CHECK_COND | cv::fisheye::CALIB_FIX_SKEW;
    cv::TermCriteria criteria(cv::TermCriteria::EPS | cv::TermCriteria::MAX_ITER, 30, 1e-6);
    cv::fisheye::calibrate(objPoints, imgPoints, size, theK, theD, cv::noArray(), cv::noArray(), flag, criteria);

    Napi::Array jsKArray = convertK(env, theK);
    Napi::Array jsDArray = convertD(env, theD);

    Napi::Object ret = Napi::Object::New(env);
    ret.Set("K", jsKArray);
    ret.Set("D", jsDArray);

    return ret;
}

Napi::Object Init(Napi::Env env, Napi::Object exports)
{
    exports.Set("undistort", Napi::Function::New(env, Undistort));
    exports.Set("calibrate", Napi::Function::New(env, Calibrate));
    return exports;
}

NODE_API_MODULE(NODE_GYP_MODULE_NAME, Init)