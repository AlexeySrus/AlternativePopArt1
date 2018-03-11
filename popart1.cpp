#include <iostream>
#include <opencv2/opencv.hpp>
#include <string>
#include <cmath>

//Объявление констант
#define MAX_EFFECT_VALUE 159
#define MAX_HALFTONE_VALUE 1000
#define DEFAULT_MAX_TRACKBAR_VALUE 100
#define MIN_WIDTH_VALUE 10
#define CIRCLE_RADIUS_COEFF 1.8 //Коэффициент alpha для вычисления радиуса кругов
#define DEFAULT_WRITE_IMAGE_PATH "test.jpg" //Путь по умолчанию для сохранения результата

using namespace std;
using namespace cv;

//Объявление глобальных констант
Mat image, half_tone_image, res_image;
uchar global_effect_value;
int global_halftone_value;
bool effect_employment;

//Альфа смешивание
uchar alpha_f(uchar color, uchar background_color, int alpha){
    return static_cast<uchar>(color*alpha / 255 + background_color*(255 - alpha) / 255);
}

//Наложение фиолетового оттенка с помощью альфа смешивания
int purple_filter(uchar& b, uchar& g, uchar& r, const uchar& C){
    b = alpha_f(b, 157, 255 - C);
    g = alpha_f(g, 30, 255 - C);
    r = alpha_f(r, 200, 255 - C);
    return EXIT_SUCCESS;
}

//Реализация первого шага алгоритма
int popart_effect(Mat& img, const uchar& C=100) {

    Mat bg_img;
    img.copyTo(bg_img);
    Mat hsv_image;
    cvtColor(img, hsv_image, CV_BGR2HSV);

#pragma omp parallel for collapse(2)
    for(auto i = 0; i < img.rows; ++i)
        for(auto j = 0; j < img.cols; ++j) {
                auto& b = img.at<Vec3b>(i,j)[0];
                auto& g = img.at<Vec3b>(i,j)[1];
                auto& r = img.at<Vec3b>(i,j)[2];

                auto& bg_b = bg_img.at<Vec3b>(i,j)[0];
                auto& bg_g = bg_img.at<Vec3b>(i,j)[1];
                auto& bg_r = bg_img.at<Vec3b>(i,j)[2];

                auto bg_avg_p = (bg_b + bg_g + bg_r) / 3;

                bg_b = 0;
                bg_g = 0;
                bg_r = 0;

                if (bg_avg_p < C)
                    bg_g = 255;
                else
                    bg_r = 255;

                b = alpha_f(b, bg_b, 255 - C);
                g = alpha_f(g, 255 - bg_g, 255 - C);
                r = alpha_f(r, bg_r, 255 - C);

                auto avg_p = (b + g + r) / 3;

                if (avg_p < 50)
                    r = alpha_f(r, 255, (100 - avg_p) * 2);

                purple_filter(b, g, r, (255 - hsv_image.at<Vec3b>(i, j)[2]) / 8);
            }

    bg_img.release();
    hsv_image.release();

    return EXIT_SUCCESS;
}

//Функция Хевисайда
int heviside(const int& x){
    return (x > 0 ? 1 : 0);
}

//Вычисления радиуса круга
int r_evalf(const int& x, const int& x_max){
    return heviside(x - MIN_WIDTH_VALUE) * static_cast<int>(x / pow(CIRCLE_RADIUS_COEFF, static_cast<double>(x) / x_max) / 2);
}

//Проверка принадлежности точки (x, y) кругу радиуса r
bool in_circle(const int& x, const int& y, const int& r){
    return x*x + y*y < r*r + 1;
}

//Отрисовка заданной окружности
int generate_circle(Mat& img, const int& i0, const int& j0, const int& rad){
    if (!rad)
        return EXIT_SUCCESS;

    if (i0 + rad + 1 > img.rows || j0 + rad + 1 > img.cols)
        return EXIT_FAILURE;

    if (i0 - rad < 0 || j0 - rad < 0)
        return EXIT_FAILURE;

    for (auto i = i0 - rad; i < i0 + rad + 1; ++i)
        for (auto j = j0 - rad; j < j0 + rad + 1; ++j)
            if (in_circle(i - i0, j - j0, rad)) {
                auto& b = img.at<Vec3b>(i,j)[0];
                auto& g = img.at<Vec3b>(i,j)[1];
                auto& r = img.at<Vec3b>(i,j)[2];

                purple_filter(b, g, r, global_effect_value);
            }

    return EXIT_SUCCESS;
}

//Реализация второго шага алгоритма
int generate_circles_effect(Mat& img, const int& width){
#pragma omp parallel for collapse(2)
    for(auto i = 0; i < img.rows / width; ++i)
        for(auto j = 0; j < img.cols / width; ++j) {
            generate_circle(img, width*i + width / 2, width*j + width / 2, r_evalf(width, MAX_HALFTONE_VALUE));
        }

    return EXIT_SUCCESS;
}

void frame_halftone_func(int pos);

//Функция для трекбара OpenCV
void frame_effect_func(int pos){
    global_effect_value = static_cast<uchar>(MAX_EFFECT_VALUE * pos / DEFAULT_MAX_TRACKBAR_VALUE);

    image.copyTo(res_image);

    if (pos == 0)
        return;

    popart_effect(res_image, global_effect_value);

    effect_employment = true;

    //Когда меняется значение параметра C, необходимо обновить действие второго шага, т.к. он зависит от данного параметра
    frame_halftone_func(global_halftone_value * DEFAULT_MAX_TRACKBAR_VALUE / MAX_HALFTONE_VALUE);
}


//Функция для трекбара OpenCV
void frame_halftone_func(int pos){
    if (!pos)
        return;

    //Обновляем значение матрицы после действия первого шага
    if (effect_employment){
        effect_employment = false;
        res_image.copyTo(half_tone_image);
    }

    half_tone_image.copyTo(res_image);

    global_halftone_value = MAX_HALFTONE_VALUE * pos / DEFAULT_MAX_TRACKBAR_VALUE;

    generate_circles_effect(res_image, global_halftone_value);
}

int popart_1(const string& im_path, const string& write_im_path=DEFAULT_WRITE_IMAGE_PATH, const bool& save=true){
    if (im_path.empty())
        return EXIT_FAILURE;

    global_effect_value = 0;
    global_halftone_value = 0;
    effect_employment = true;

    image = imread(im_path, IMREAD_COLOR);

    image.copyTo(res_image);
    image.copyTo(half_tone_image);

    const int window_h = 1000;
    int current_position_effect = 0, max_position_effect = DEFAULT_MAX_TRACKBAR_VALUE;
    int current_position_halftone = 0, max_position_halftone = DEFAULT_MAX_TRACKBAR_VALUE;

    string window = to_string(res_image.cols) + "x" + to_string(res_image.rows), track1_name = "Effect", track2_name = "Halftone";

    namedWindow(window, WINDOW_NORMAL);
    resizeWindow(window, window_h*16/9, window_h);

    cvCreateTrackbar(track1_name.c_str(), window.c_str(), &current_position_effect, max_position_effect, frame_effect_func);
    cvCreateTrackbar(track2_name.c_str(), window.c_str(), &current_position_halftone, max_position_halftone, frame_halftone_func);

    //Выводим результат пока не нажата клавиша "Esc"
    while(true){
        imshow(window, res_image);

        auto c = waitKey(33);
        if (c == 27)
            break;
    }

    //Если инициализирован флаг сохраняем результат
    if (save)
        imwrite(write_im_path, res_image);

    image.release();
    res_image.release();

    return EXIT_FAILURE;
}

int main(int argc, char** argv) {

    string im_path;

    if (argc < 2){
        //Если не передан путь до изображения, просим ввести с консоли
        cout << "Input image path: " << endl;
        cin >> im_path;
    } else
        im_path = argv[1];

    popart_1(im_path, argc > 2 ? argv[2] : DEFAULT_WRITE_IMAGE_PATH);

    return EXIT_SUCCESS;
}