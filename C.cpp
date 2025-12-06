#pragma once

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <random>
#include <unordered_map>
#include <vector>

/*

# C. Тындекс.Экспресс

## Легенда
В мобильном клиенте интернет-магазина "Тындекс.Экспресс" необходимо реализовать ленту лучших
товаров. Лента представляет собой бесконечный список карточек товаров, который можно прокручивать.
На каждой карточке представлена информация о товаре.

С каждым товаром связан уникальный идентификатор, а еще его рейтинг. Уникальный идентификатор -
положительное целое число (SERIAL из PostgreSQL), рейтинг - положительное целое число длины 16 бит
(такое значение им приходит от какой-то модели).

В списке товары отсортированы по возрастанию рейтинга, а если рейтинг совпадает, то выше будет товар
с наименьшим идентификатором. Получается, что тем меньше рейтинг, тем выше (лучше) товар в списке.
Пользователь должен иметь возможность быстро находить в списке товар по идентификатору или
порядковому номеру в ленте лучших товаров. То есть при заполнении некоторой формы, осуществляется
переход на нужную карточку в списке.

Конечно же, товаров в системе может быть достаточно много, потому загружать информацию о всех
товарах в оперативную память устройства может быть не очень удобно, поэтому пришлось реализовать
некоторый аналог пагинации, только для бесконечно прокручивающихся списков.

Кроме этого, множество товаров на маркетплейсе "Тындекс.Экспресс" постоянно изменяется - там
постоянно появляются новые, исчезают старые товары, а также меняется рейтинг товаров.

Инженеры из команды мобильной разработки сообщили, что им для реализации обсуждаемой
функциональности будет достаточно API метода, возвращающего список товаров заданной длины,
начинающийся с товара с заданным идентификатором или позицией в топе.

Естественно, хотим, чтобы все работало быстро - как методы получения, так и методы обновления
данных. Ожидаемая нагрузка: редкие удаления, добавления, частые обновления, очень частые запросы на
получение. Однако, нужно быть готовым к атакам с иным паттерном нагрузки.

Наши тестировщики уже разработали модульные тесты для нашего нового компонента. Осталось лишь вам
реализовать интерфейс! Подразумевается, что входные данные корректны.

## Формат ввода
Первая строка входных данных содержит целое число n (0 ≤ n ≤ 1000000) — количество операций.
Следующие n строк описывают операции.

Строки имеют вид:
- i <id> <score>. Добавление товара с уникальным идентификатором id и рейтингом score.
Гарантируется, уникальность идентификатора, а также 1 ≤ id ≤ 10000000, 1 ≤ score ≤ 65535.
- u <id> <score>. Обновление товара. Гарантируется, что товар с заданным идентификатором был
добавлен в коллекцию.
- r <id>. Удаление товара. Гарантируется, что товар с заданным идентификатором был добавлен в
коллекцию и после удаления больше никогда не появится.
- p <position> <limit>. Получение limit товаров, начиная с позиции в ленте position. Гарантируется,
что 0 ≤ position < length(feed), 1 ≤ limit ≤ 16.
- g <id> <limit>. Получение limit товаров, начиная с товара, имеющего уникальный идентификатор id.
Гарантируется, что товар есть в коллекции.

## Формат вывода
Сперва в отдельной строке выведите Started.

Далее для каждой из n операций в отдельной строке выведите:
- Added после добавления.
- Updated после обновления.
- Removed после удаления.
- AtPos: [(item_id ',' ' ')+] после получения товаров по позиции в списке.
- AtId: [(item_id ',' ' ' )+] после получения товаров по идентификатору товара.

## Описание решения
В качестве реализации заданной по заданию структуры данных было выбрано Treap-дерево (описание
реализации можно найти в открытых источниках). Упорядочивание происходит по ключу (score, id) - как
по условию задачи. Приоритеты вершин задаются случайно.

Чтобы решить поставленные в задаче условия были сделаны следующие изменения:
1. Каждая вершина имеет указатель на родителя (необходимо для выполнения операций, содержащих limit)
2. Каждая вершина имеет дополнительное поле subtree_size, равное количеству вершин в поддереве
(необходимо для выполнения операции поиска, начиная с заданной позиции)

Дополнительно поддерживается структура Map, хранящая соответствие идентификатора вершины указателю
на вершину в дереве (необходимо для выполнения поиска, начиная с заданного идентификатора).

Операций вставки, обновления, удаления происходят в соответствии с описанием для структуры Treap. Во
время выполнения указанных операций все описанные ранее параметры поддерживаются в актуальном
состоянии.

Детальное описание каждой операции кажется бессмысленным.

## Асимптотика решения
Время: O(n ^ 2)
Память: O(n)

Разумно предположить, что, учитывая "случайность" природы Treap, "честное" время выполнения
посчитать практически невозможно. Указана асимптотика выполнеиня программы в "худшем" случае.

Асимптотика по времени операций поиска, вставки, обновления, удаления соответствует асимптотикам
поиска, вставки, обновления, удаления в структуре Treap. Актуализация описанных параметров занимает
O(1).

*/

namespace youndex::express {

struct Item {
  std::uint64_t id;
  std::uint16_t score;
};

class ItemFeed final {
public:
  ItemFeed() : root_(nullptr) {
  }
  ~ItemFeed() {
    Clear(root_);
  }

  [[nodiscard]] std::vector<std::uint64_t> GetAtPosition(std::size_t position, std::size_t limit)
      const {
    assert(position < Size());
    assert(1 <= limit && limit <= 16);

    std::vector<std::uint64_t> result;
    result.reserve(limit);

    Node* node = Kth(root_, position);
    for (std::size_t i = 0; i < limit && node != nullptr; ++i) {
      result.push_back(node->item.id);
      node = Successor(node);
    }
    return result;
  }

  [[nodiscard]] std::vector<std::uint64_t> GetAtItemWithId(std::uint64_t uid, std::size_t limit)
      const {
    assert(Contains(uid));
    assert(1 <= limit && limit <= 16);

    std::vector<std::uint64_t> result;
    result.reserve(limit);

    Node* node = id_map_.at(uid);
    for (std::size_t i = 0; i < limit && node != nullptr; ++i) {
      result.push_back(node->item.id);
      node = Successor(node);
    }
    return result;
  }

  void Add(Item item) {
    assert(!Contains(item.id));
    root_ = Insert(root_, new Node(item), nullptr);
  }

  void Update(Item item) {
    assert(Contains(item.id));
    Remove(item.id);
    Add(item);
  }

  void Remove(std::uint64_t uid) {
    assert(Contains(uid));
    Item key = id_map_.at(uid)->item;
    root_ = Erase(root_, key);
  }

private:
  struct Node {
    Item item;
    Node* left = nullptr;
    Node* right = nullptr;
    Node* parent = nullptr;
    std::size_t subtree_size = 1;
    int priority;
    explicit Node(const Item& it) : item(it), priority(rand_dist(rand_eng)) {
    }
  };

  Node* root_;
  std::unordered_map<std::uint64_t, Node*> id_map_;

  static bool KeyLess(const Item& a, const Item& b) {
    return a.score != b.score ? a.score < b.score : a.id < b.id;
  }

  static bool KeyEqual(const Item& a, const Item& b) {
    return a.score == b.score && a.id == b.id;
  }

  [[nodiscard]] bool Contains(std::uint64_t uid) const {
    return id_map_.find(uid) != id_map_.end();
  }

  [[nodiscard]] std::size_t Size() const {
    return SubtreeSize(root_);
  }

  static std::size_t SubtreeSize(const Node* n) {
    return n ? n->subtree_size : 0;
  }

  static void Update(Node* n) {
    if (!n)
      return;
    n->subtree_size = 1 + SubtreeSize(n->left) + SubtreeSize(n->right);
    if (n->left)
      n->left->parent = n;
    if (n->right)
      n->right->parent = n;
  }

  // Treap split/merge
  static void Split(Node* t, const Item& key, Node*& left, Node*& right) {
    if (!t) {
      left = right = nullptr;
      return;
    }
    if (KeyLess(t->item, key) || KeyEqual(t->item, key)) {
      Split(t->right, key, t->right, right);
      left = t;
    } else {
      Split(t->left, key, left, t->left);
      right = t;
    }
    Update(t);
  }

  static Node* Merge(Node* left, Node* right) {
    if (!left)
      return right;
    if (!right)
      return left;
    if (left->priority > right->priority) {
      left->right = Merge(left->right, right);
      Update(left);
      return left;
    } else {
      right->left = Merge(left, right->left);
      Update(right);
      return right;
    }
  }

  Node* Insert(Node* root, Node* node, Node* parent) {
    if (!root) {
      node->parent = parent;
      id_map_[node->item.id] = node;
      return node;
    }
    if (node->priority > root->priority) {
      Split(root, node->item, node->left, node->right);
      node->parent = parent;
      if (node->left)
        node->left->parent = node;
      if (node->right)
        node->right->parent = node;
      Update(node);
      id_map_[node->item.id] = node;
      return node;
    } else if (KeyLess(node->item, root->item)) {
      root->left = Insert(root->left, node, root);
    } else {
      root->right = Insert(root->right, node, root);
    }
    Update(root);
    return root;
  }

  Node* Erase(Node* root, const Item& key) {
    if (!root)
      return nullptr;
    if (KeyEqual(root->item, key)) {
      id_map_.erase(root->item.id);
      Node* merged = Merge(root->left, root->right);
      if (merged)
        merged->parent = root->parent;
      delete root;
      return merged;
    } else if (KeyLess(key, root->item)) {
      root->left = Erase(root->left, key);
      if (root->left)
        root->left->parent = root;
    } else {
      root->right = Erase(root->right, key);
      if (root->right)
        root->right->parent = root;
    }
    Update(root);
    return root;
  }

  static Node* Kth(Node* root, std::size_t k) {
    if (!root)
      return nullptr;
    std::size_t left_size = SubtreeSize(root->left);
    if (k < left_size)
      return Kth(root->left, k);
    else if (k == left_size)
      return root;
    else
      return Kth(root->right, k - left_size - 1);
  }

  // Successor using parent pointers
  static Node* Successor(Node* node) {
    if (!node)
      return nullptr;
    if (node->right) {
      node = node->right;
      while (node->left)
        node = node->left;
      return node;
    }
    Node* parent = node->parent;
    while (parent && node == parent->right) {
      node = parent;
      parent = parent->parent;
    }
    return parent;
  }

  static void Clear(Node* node) {
    if (!node)
      return;
    Clear(node->left);
    Clear(node->right);
    delete node;
  }

  // Random engine for treap priority
  static inline std::mt19937 rand_eng{std::random_device{}()};
  static inline std::uniform_int_distribution<int> rand_dist{1, 1'000'000'000};
};

}  // namespace youndex::express
