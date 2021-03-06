# Alternative realization PopArt1
Реализация эффекта, близкого к PopArt1 с сайта BeFunky
---
## Описание алгоритма
---
Реализация данного эффекта состоит из двух частей:
1.  Работа с BGR и HSV пространствами
2.  Отрисовка кругов на изображении с помощью альфа смешивания
---
### Рассмотрим шаги первого этапа алгоритма
* Эффект зависит от передаваемого значения C
* На входе имеется изображение в цветовом пространстве BGR.
* Генерируется фон следующим образом:
  1. Вычисляется интенсивность пикселя (среднее значение цветов r, g, b)
  2. Если интенсивность меньше заданного значения (C), то соответственная точка фона будет иметь значение (255, 0, 0) (BGR), иначе (0, 0, 255)
* Применяем альфа смешивание ко всем цветам пикселя:
  1. `B = AlphaSmooth(B, Background_B, 255 - C)`
  2. `G = AlphaSmooth(G, 255 - Background_G, 255 - C)`
  3. `R = AlphaSmooth(R, Background_R, 255 - C)`
* Вычисляем текущее значение интенсивнсости пикселя и изменяем значение красного следующим образом:

  `Если AVG(B, G, R) < 50, то R = AlphaSmooth(R, 255, (100 - AVG(B, G, R)) * 2)`
* Применяем альфа смешивание следующего вида:
  + `V - значение пикселя из HSV`
  + `B = AlphaSmooth(B, 157, 255 - (255 - V) / 8)`
  + `G = AlphaSmooth(G, 30, 255 - (255 - V) / 8)`
  + `R = AlphaSmooth(R, 200, 255 - (255 - V) / 8)`
---
### Второй этап алгоритма
* Данный эффект зависит от значения W (два радиуса круга) и предыдущего параметра C
* Отрисовываем круги с помощью альфа смешивание следующим образом:
  1. Вычисление радиуса:
  
  <a href="http://www.codecogs.com/eqnedit.php?latex=$r(\omega)\&space;=\&space;\Theta(\omega&space;-&space;\omega_0)*\omega*\alpha^{-\omega&space;/&space;\omega_{max}}&space;/&space;2$" target="_blank"><img src="http://latex.codecogs.com/gif.latex?$r(\omega)\&space;=\&space;\Theta(\omega&space;-&space;\omega_0)*\omega*\alpha^{-\omega&space;/&space;\omega_{max}}&space;/&space;2$" title="$r(\omega)\ =\ \Theta(\omega - \omega_0)*\omega*\alpha^{-\omega / \omega_{max}} / 2$" /></a>, где <a href="http://www.codecogs.com/eqnedit.php?latex=\omega" target="_blank"><img src="http://latex.codecogs.com/gif.latex?\omega" title="\omega" /></a> - это значение параметра W, а <a href="http://www.codecogs.com/eqnedit.php?latex=\omega_0" target="_blank"><img src="http://latex.codecogs.com/gif.latex?\omega_0" title="\omega_0" /></a> - минимальное значение, при меньшем значении круги не отрисовываются, <a href="http://www.codecogs.com/eqnedit.php?latex=\omega_{max}" target="_blank"><img src="http://latex.codecogs.com/gif.latex?\omega_{max}" title="\omega_{max}" /></a> - максимальное значение параметра, <a href="http://www.codecogs.com/eqnedit.php?latex=\alpha" target="_blank"><img src="http://latex.codecogs.com/gif.latex?\alpha" title="\alpha" /></a> - параметр, <a href="http://www.codecogs.com/eqnedit.php?latex=\Theta" target="_blank"><img src="http://latex.codecogs.com/gif.latex?\Theta" title="\Theta" /></a> - функция Хевисайда
  
  2. Изображение бъётся на квадраты со стороной W и в каждом отрисовывается круг радиуса <a href="http://www.codecogs.com/eqnedit.php?latex=r(\omega)" target="_blank"><img src="http://latex.codecogs.com/gif.latex?r(\omega)" title="r(\omega)" /></a> с центром, совпадающим с центром квадрата следующим образом:
    + `B = AlphaSmooth(B, 157, 255 - C)`
    + `G = AlphaSmooth(G, 30, 255 - C)`
    + `R = AlphaSmooth(R, 200, 255 - C)`
---
## Примеры работы алгоритма
---
### `Оригинальное изображение, C = 0, W = 0`
![](/images/2560x1440.jpg)
---
### `C = 71, W = 0`
![](/images/test_71_0.jpg)
---
### `C = 45, W = 4`
![](/images/test_45_4.jpg)
---
### `C = 45, W = 48`
![](/images/test_45_48.jpg)
---
## Примечания
* Функция альфа смешивания имеет вид:

  <a href="http://www.codecogs.com/eqnedit.php?latex=AlphaSmooth(c_1,&space;c_2,&space;\alpha)\&space;=&space;\&space;c_1&space;*&space;\alpha&space;/&space;255&space;&plus;&space;c_2&space;*&space;(255&space;-&space;\alpha)&space;/&space;255" target="_blank"><img src="http://latex.codecogs.com/gif.latex?AlphaSmooth(c_1,&space;c_2,&space;\alpha)\&space;=&space;\&space;c_1&space;*&space;\alpha&space;/&space;255&space;&plus;&space;c_2&space;*&space;(255&space;-&space;\alpha)&space;/&space;255" title="AlphaSmooth(c_1, c_2, \alpha)\ = \ c_1 * \alpha / 255 + c_2 * (255 - \alpha) / 255" /></a>
  
* Файл popart1.cpp содержит реализацию алгоритма
* Алгоритм реализован на языке C++, стандарта 11
* Требуется поддержка компилятром стандарта OpenMP не ниже 2.0
* Пример `Cmake` файла для компиляции:
```cmake
cmake_minimum_required(VERSION 3.8)
project(pop_art1)

set(CMAKE_CXX_COMPILER "g++")
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_FLAGS "-O3 -fopenmp")

find_package(OpenCV REQUIRED)

set(SOURCE_FILES popart1.cpp)
add_executable(pop_art1 ${SOURCE_FILES})
target_link_libraries(pop_art1 ${OpenCV_LIBS})
```
* Программа сохраняет последнее состояние изображения
