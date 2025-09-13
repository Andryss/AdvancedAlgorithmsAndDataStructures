#include <cstdint>
#include <iostream>
#include <vector>

/*

A. K-ый ноль

Формат ввода
В первой строке вводится одно натуральное число N (1 ≤ N ≤ 200000) — количество чисел в массиве. Во
второй строке вводятся N чисел от 0 до 100000 — элементы массива. В третьей строке вводится одно
натуральное число M (1 ≤ M ≤ 200000) — количество запросов. Каждая из следующих M строк представляет
собой описание запроса. Сначала вводится одна буква, кодирующая вид запроса (s — вычислить индекс
k-го нуля, u — обновить значение элемента). Следом за s вводится три числа — левый и правый концы
отрезка и число k (1 ≤ k ≤ N). Следом за u вводятся два числа — номер элемента и его новое значение.

Формат вывода
Для каждого запроса s выведите результат. Все числа выводите в одну строку через пробел. Если
нужного числа нулей на запрашиваемом отрезке нет, выводите −1 для данного запроса.

*/

class SegmentedTree {
  std::vector<uint32_t> array;

public:
  SegmentedTree(std::vector<uint32_t> source) {
    array = source;
  }

  void update(size_t idx, uint32_t new_value) {
    // TODO
  }

  size_t search(size_t left, size_t right, uint32_t k) {
    // TODO
  }
};

int main() {
  uint32_t n;
  std::cin >> n;

  std::vector<uint32_t> array(n);

  uint32_t value;
  while (n-- > 0) {
    std::cin >> value;
    array.push_back(value);
  }

  SegmentedTree tree(array);

  uint32_t m;
  std::cin >> m;

  while (m-- > 0) {
    char command;
    std::cin >> command;

    if (command == 'u') {
      size_t idx;
      std::cin >> idx;

      uint32_t new_value;
      std::cin >> new_value;

      tree.update(idx, new_value);
    } else {
      size_t left;
      std::cin >> left;

      size_t right;
      std::cin >> right;

      uint32_t k;
      std::cin >> k;

      size_t idx = tree.search(left, right, k);
      std::cout << idx << '\n';
    }
  }

  return 0;
}