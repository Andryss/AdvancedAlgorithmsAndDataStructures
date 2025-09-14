#include <cstdint>
#include <iostream>
#include <vector>

/*

# A. K-ый ноль

## Формат ввода
В первой строке вводится одно натуральное число N (1 ≤ N ≤ 200000) — количество чисел в массиве. Во
второй строке вводятся N чисел от 0 до 100000 — элементы массива. В третьей строке вводится одно
натуральное число M (1 ≤ M ≤ 200000) — количество запросов. Каждая из следующих M строк представляет
собой описание запроса. Сначала вводится одна буква, кодирующая вид запроса (s — вычислить индекс
k-го нуля, u — обновить значение элемента). Следом за s вводится три числа — левый и правый концы
отрезка и число k (1 ≤ k ≤ N). Следом за u вводятся два числа — номер элемента и его новое значение.

## Формат вывода
Для каждого запроса s выведите результат. Все числа выводите в одну строку через пробел. Если
нужного числа нулей на запрашиваемом отрезке нет, выводите −1 для данного запроса.

## Описание решения
Разобьем весь массив на блоки одинакового размера BLOCK_SIZE. Для каждого из блоков заранее
посчитаем количество 0 в нем. Для ответа на запрос update нужно только вычислить изменяемый блок и
обновить сумму 0 в нем (если ноль появился или затерся новым значением).

Время ответа на запрос обновления будет определяться сложностью расчета номера блока для
определенного индекса.

Для ответа на запрос search нужно итерироваться по всем блокам в промежутке от left до right и
вычислить блок, в котором находится нужный k-тый 0. Для найденного блока честно пройтись по всем
элементам и найти индекс нужного k-того.

Для случаев, когда left или right попадают НЕ на границу блоков, честно итерируемся по тем элементам
блока, которые попадут в промежуток [left, right]. Максимальное количество таких "честных" итераций
для одного запроса - 2 (один с левой стороны, второй с правой).

Размер блока удобнее взять числом, являющимся степенью двойки, так как в таком случае вычислить
номер блока для индекса и индекс начала определенного блока можно путем битового сдвига, что
происходит сравнительно быстро и не вызовет проблем с производительностью.

Оценим время ответа на запрос обновления: 2 * BLOCK_SIZE + BLOCKS_COUNT = 2 * BLOCK_SIZE + (N /
BLOCK_SIZE). Наименьшее значение выведенная функция примет при BLOCK_SIZE примерно равному
квадратному корню из N. Таким образом, BLOCK_SIZE лучше всего брать ближайшей степенью двойки к
квадратному корню из N. При условиях, данных в задаче - 256 или 512.

## Асимптотика решения
Время: O(M * sqrt(N))
Память: O(N)

*/

constexpr uint32_t BlockSizeShift = 7;  // размер блока равен 2^BlockSizeShift

class SegmentedArray {
  uint32_t block_size_;
  std::vector<uint32_t> array_;
  std::vector<uint16_t> zeros_;

public:
  explicit SegmentedArray(const std::vector<uint32_t>& source)
      : block_size_(static_cast<uint32_t>(1) << BlockSizeShift), array_(source) {
    const uint32_t size = array_.size();
    uint32_t blocks_count = size >> BlockSizeShift;
    if ((size & (block_size_ - 1)) > 0) {
      blocks_count++;
    }

    zeros_ = std::vector<uint16_t>(blocks_count, 0);

    for (uint32_t i = 0; i < size; i++) {
      if (array_[i] == 0) {
        zeros_[BlockOfElement(i)]++;
      }
    }
  }

  void Update(const uint32_t index, const uint32_t new_value) {
    const uint32_t idx = index - 1;

    const uint32_t old_value = array_[idx];

    // Если новое значение равно старому, то обновлять ничего не нужно
    if (old_value == new_value) {
      return;
    }

    array_[idx] = new_value;

    // Уменьшаем счетчик 0, если затерли значение 0
    if (old_value == 0) {
      zeros_[BlockOfElement(idx)]--;
    }

    // Увеличиваем счетчик 0, если записываем новый 0
    if (new_value == 0) {
      zeros_[BlockOfElement(idx)]++;
    }
  }

  int32_t Search(const uint32_t left_bound, const uint32_t right_bound, const uint32_t k_value) {
    if (left_bound > right_bound) {
      return -1;
    }

    const uint32_t left = left_bound - 1;
    const uint32_t right = right_bound - 1;

    const uint32_t left_block = BlockOfElement(left);
    const uint32_t right_block = BlockOfElement(right);

    // Если границы попали на один блок, то считаем честно через цикл
    if (left_block == right_block) {
      uint32_t zeros_count = 0;
      for (uint32_t i = left; i <= right; i++) {
        if (array_[i] == 0 && ++zeros_count == k_value) {
          return static_cast<int32_t>(i + 1);
        }
      }
      return -1;
    }

    uint32_t zeros_count = 0;
    uint32_t idx = left;
    uint32_t i_block = left_block;

    // Считаем 0 в правой части блока, в который попала левая граница
    if (FirstIndexOfBlock(left_block) != left) {
      while (i_block == left_block) {
        if (array_[idx] == 0 && ++zeros_count == k_value) {
          return static_cast<int32_t>(idx + 1);
        }
        idx++;
        i_block = BlockOfElement(idx);
      }
    }

    // Считаем 0 в блоках между блоками, в которые попали левая и правая границы
    while (i_block < right_block) {
      // Если в промежуточном блоке нашли нужный k-тый 0
      if (zeros_count + zeros_[i_block] >= k_value) {
        // Честно итерируемся по блоку
        idx = FirstIndexOfBlock(i_block);
        while (zeros_count < k_value) {
          if (array_[idx] == 0) {
            zeros_count++;
          }
          idx++;
        }
        return static_cast<int32_t>(idx);
      }

      zeros_count += zeros_[i_block];
      i_block++;
    }

    // Честно итерируемся по блоку, в который попала правая граница
    for (idx = FirstIndexOfBlock(right_block); idx <= right; idx++) {
      if (array_[idx] == 0 && ++zeros_count == k_value) {
        return static_cast<int32_t>(idx + 1);
      }
    }

    // Нужного k-того 0 на промежутке не нашли
    return -1;
  }

private:
  static uint32_t BlockOfElement(const uint32_t index) {
    return index >> BlockSizeShift;
  }

  static uint32_t FirstIndexOfBlock(const uint32_t block) {
    return block << BlockSizeShift;
  }
};

int main() {
  // Немного магии, которую я честно не понимаю
  std::ios::sync_with_stdio(false);
  std::cin.tie(nullptr);

  uint32_t n_value = 0;
  std::cin >> n_value;

  // Считываем массив
  std::vector<uint32_t> array(n_value);
  uint32_t value = 0;
  for (uint32_t i = 0; i < n_value; i++) {
    std::cin >> value;
    array[i] = value;
  }

  SegmentedArray tree(array);

  uint32_t m_value = 0;
  std::cin >> m_value;
  std::string output;

  while (m_value-- > 0) {
    char command = 0;
    std::cin >> command;

    if (command == 'u') {
      // Обрабатываем команду обновления элемента
      uint32_t idx = 0;
      std::cin >> idx;

      uint32_t new_value = 0;
      std::cin >> new_value;

      tree.Update(idx, new_value);
    } else {
      // Обрабатываем команду поиска k-того 0
      uint32_t left = 0;
      std::cin >> left;

      uint32_t right = 0;
      std::cin >> right;

      uint32_t k_value = 0;
      std::cin >> k_value;

      const int32_t idx = tree.Search(left, right, k_value);
      output += std::to_string(idx) + '\n';
    }
  }

  std::cout << output;

  return 0;
}

/*

10
0 1 2 3 4 5 6 7 8 9
6
u 5 0
u 10 0
s 1 10 1
s 1 10 2
s 1 10 3
s 1 10 4

*/
