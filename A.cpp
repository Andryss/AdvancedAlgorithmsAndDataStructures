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

const uint16_t BLOCK_SIZE = 256;
const uint16_t BLOCK_SIZE_SHIFT = 8;

class SegmentedArray {
  std::vector<uint32_t> array;
  std::vector<uint16_t> zeros;

public:
  SegmentedArray(std::vector<uint32_t> source) {
    array = source;

    size_t size = array.size();
    size_t blocks_count = size >> BLOCK_SIZE_SHIFT;
    if ((size & BLOCK_SIZE) > 0) {
      blocks_count++;
    }

    zeros = std::vector<uint16_t>(blocks_count, 0);

    for (size_t i = 0; i < size; i++) {
      if (array[i] == 0) {
        zeros[block_of_element(i)]++;
      }
    }
  }

  void update(size_t idx, uint32_t new_value) {
    idx--;

    uint32_t old_value = array[idx];

    if (old_value == new_value) {
      return;
    }

    array[idx] = new_value;

    if (old_value == 0) {
      zeros[block_of_element(idx)]--;
    }

    if (new_value == 0) {
      zeros[block_of_element(idx)]++;
    }
  }

  size_t search(size_t left, size_t right, uint32_t k) {
    if (left > right) {
      return -1;
    }

    left--; right--;

    size_t left_block = block_of_element(left);
    size_t right_block = block_of_element(right);

    // Если границы попали на один блок, то считаем честно через цикл
    if (left_block == right_block) {
      uint32_t zeros_count = 0;
      for (size_t i = left; i <= right; i++) {
        if (array[i] == 0 && (++zeros_count == k)) {
          return ++i;
        }
      }
      return -1;
    }

    uint32_t zeros_count = 0;
    size_t i = left;
    size_t i_block = left_block;

    // Считаем 0 в правой части блока, в который попала левая граница
    while (i_block == left_block) {
      if (array[i] == 0 && (++zeros_count == k)) {
        return ++i;
      }
      i++;
      i_block = block_of_element(i);
    }

    // Считаем 0 в блоках между блоками, в которые попали левая и правая границы
    while (i_block < right_block) {
      // Если в промежуточном блоке нашли нужный k-тый 0
      if (zeros_count + zeros[i] >= k) {
        // Честно итерируемся по блоку
        i = first_index_of_block(i_block);
        while (zeros_count < k) {
          if (array[i] == 0) {
            zeros_count++;
          }
          i++;
        }
        return i;
      }

      zeros_count += zeros[i];
      i_block++;
    }

    // Честно итерируемся по блоку, в который попала правая граница
    for (i = first_index_of_block(right_block); i <= right; i++) {
      if (array[i] == 0 && (++zeros_count == k)) {
        return ++i;
      }
    }

    // Нужного k-того 0 на промежутке не нашли
    return -1;
  }

private:
  size_t block_of_element(size_t index) {
    return index >> BLOCK_SIZE_SHIFT;
  }

  size_t first_index_of_block(size_t block) {
    return block << BLOCK_SIZE_SHIFT;
  }
};

int main() {
  uint32_t n;
  std::cin >> n;

  // Считываем массив
  std::vector<uint32_t> array(n);
  uint32_t value;
  while (n-- > 0) {
    std::cin >> value;
    array.push_back(value);
  }

  SegmentedArray tree(array);

  uint32_t m;
  std::cin >> m;

  while (m-- > 0) {
    char command;
    std::cin >> command;

    if (command == 'u') {
      // Обрабатываем команду обновления элемента
      size_t idx;
      std::cin >> idx;

      uint32_t new_value;
      std::cin >> new_value;

      tree.update(idx, new_value);
    } else {
      // Обрабатываем команду поиска k-того 0
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