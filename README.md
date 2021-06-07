# Globalizer Lite — программная система для параллельного поиска глобально-оптимальных решений
## Краткое описание
Программная система `Globalizer Lite` предназначена для решения трудоемких задач многомерной многоэкстремальной глобальной оптимизации.

Алгоритмическую основу системы составляет блочная многошаговая схема редукции размерности, комбинирующая использование разверток типа кривой Пеано и многошаговую схему редукции, что позволяет эффективно распараллеливать вычисления на большое число процессоров/ядер. 

Параллельная версия блочной многошаговой схемы реализована с использованием библиотеки `MPI`.

## Ограничения
1. Система `Globalizer Lite` предназначена для решения задач многомерной глобальной однокритериальной оптимизации.
1. Размерность решаемых задач ограничена пятью параметрами.
1. В текущей версии системы рассматриваются только задачи  безусловной оптимизации.
1. Метод решения основан на редукции размерности с использованием разверток.
1. Редукция выполняется с использованием одиночной развертки.
1. Распараллеливание выполняется по характеристикам.
1. Постановка задачи оптимизации с точки зрения пользователя системы состоит в подключении динамической библиотеки, в которой должна быть реализована оптимизируемая функция.
1. Система представляет собой консольное приложение без графического интерфейса.
