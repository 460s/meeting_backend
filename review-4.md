# Виды ошибок и отладка
Undefined Behaviour - неопределенное поведение, результат программы зависит от случайных факторов - компилятор, состояние памяти.
Segmentation fault - неверная работа с памятью, обращение к неинициализированным данным, обращение к освобожденной переменной.
Exception - исключение, ошибка созданная вами или используемой библиотекой.
Неожиданное поведение - запускаете, а программа не делает, то что нужно.

## Отладка
* coredump
  - ulimit -c unlimited
  - sysctl kernel.core_pattern
  - sudo sysctl -w kernel.core_pattern=core
* Пишем в лог (трассируем) те места через которые прошло управление, чтобы локализовать ошибку
* Запуск отладчика - gdb

# Упредительные меры
* Следование рекомендациям - https://github.com/isocpp/CppCoreGuidelines и https://google.github.io/styleguide/cppguide.html
* Статический анализ - ошибки и предупреждения компилятора clang-tidy, cppcheck, pvs-studio
  - python3 /usr/bin/run-clang-tidy -p cmake-build-debug
  - clang-tidy-6.0 -header-filter=^cmake-build-debug/.* -p=cmake-build-debug /home/sikroz/src/meeting_backend/handlers/src/factory.cpp
* Дополнительные проверки времени выполнения sanitizer, valgrind
  - export FLAGS='-fsanitize=address -fno-omit-frame-pointer'
  - export CXXFLAGS="$FLAGS"
  - export CFLAGS="$FLAGS"
  - conan install --build missing --settings build_type=Debug ..
  - set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address -fno-omit-frame-pointer -g") в CMakeLists.txt


```cpp
// leak.cpp
#include <iostream>
int main() {
	int *p = new int(5);
	std::cout << *p << std::endl;
	return 0;
}
```
`clang++ -g -fsanitize=address leak.cpp`

```cpp
// array.cpp
int main() {
        char *a = "0123456789";
        a[10] = '1';
}
```

# Домашнее задание
 * Используя документацию https://pocoproject.org/slides/110-Logging.pdf прикрутить к нашему серверу логирование
 * Попробовать clang-tidy на проекте