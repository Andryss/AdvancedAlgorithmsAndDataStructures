#pragma once

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <unordered_map>
#include <vector>

/*

# B. Количество единиц в двоичной записи

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
TBD

## Асимптотика решения
Время: TBD
Память: TBD

*/

namespace youndex::express {

struct Item {
  std::uint64_t id;
  std::uint16_t score;
};

/// NB: Is not thread safe.
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

    Node* start = Kth(root_, position);  // 0-based
    Node* cur = start;
    for (std::size_t i = 0; i < limit && cur != nullptr; ++i) {
      result.push_back(cur->item.id);
      cur = Successor(cur);
    }
    return result;
  }

  [[nodiscard]] std::vector<std::uint64_t> GetAtItemWithId(std::uint64_t uid, std::size_t limit)
      const {
    assert(Contains(uid));
    assert(1 <= limit && limit <= 16);

    std::vector<std::uint64_t> result;
    result.reserve(limit);

    auto it = id_map_.find(uid);
    Node* cur = it->second;
    for (std::size_t i = 0; i < limit && cur != nullptr; ++i) {
      result.push_back(cur->item.id);
      cur = Successor(cur);
    }
    return result;
  }

  void Add(Item item) {
    assert(!Contains(item.id));
    Node* z = new Node(item);
    Node* y = nullptr;
    Node* x = root_;
    while (x != nullptr) {
      y = x;
      ++(x->subtree_size);
      if (KeyLess(z->item, x->item)) {
        x = x->left;
      } else {
        x = x->right;
      }
    }
    z->parent = y;
    if (y == nullptr) {
      root_ = z;
    } else if (KeyLess(z->item, y->item)) {
      y->left = z;
    } else {
      y->right = z;
    }
    id_map_.emplace(item.id, z);
  }

  void Update(Item item) {
    assert(Contains(item.id));
    // remove old, insert new with same id (per instructions)
    Remove(item.id);
    Add(item);
  }

  void Remove(std::uint64_t uid) {
    assert(Contains(uid));
    Node* z = id_map_[uid];

    // If z has two children, we will copy successor's item into z and delete successor.
    // Keep track of original id to erase from map later.
    std::uint64_t original_id = z->item.id;

    Node* node_to_delete = nullptr;
    if (z->left == nullptr || z->right == nullptr) {
      node_to_delete = z;
    } else {
      node_to_delete = Minimum(z->right);
      // copy successor's item into z
      // update id_map for successor's id to point to z
      std::uint64_t succ_id = node_to_delete->item.id;
      if (succ_id != z->item.id) {
        // Move successor's item to z and update map
        z->item = node_to_delete->item;
        id_map_[z->item.id] = z;
      }
    }

    Node* child = (node_to_delete->left != nullptr) ? node_to_delete->left : node_to_delete->right;
    Transplant(node_to_delete, child);

    // Update subtree sizes up the ancestor chain starting from the parent of the place where
    // deletion happened
    Node* update_from = node_to_delete->parent;
    while (update_from != nullptr) {
      update_from->subtree_size =
          1 + SubtreeSize(update_from->left) + SubtreeSize(update_from->right);
      update_from = update_from->parent;
    }

    // erase original id mapping
    id_map_.erase(original_id);

    // if we deleted a successor node but copied its id into z, we already updated id_map for that
    // id above. delete the node memory
    delete node_to_delete;
  }

private:
  struct Node {
    Item item;
    Node* left = nullptr;
    Node* right = nullptr;
    Node* parent = nullptr;
    std::size_t subtree_size = 1;
    explicit Node(Item it) : item(it) {
    }
  };

  Node* root_;
  std::unordered_map<std::uint64_t, Node*> id_map_;

  static bool KeyLess(const Item& a, const Item& b) {
    if (a.score != b.score)
      return a.score < b.score;
    return a.id < b.id;
  }

  [[nodiscard]] bool Contains(std::uint64_t uid) const {
    assert(1 <= uid);
    return id_map_.find(uid) != id_map_.end();
  }

  [[nodiscard]] std::size_t Size() const {
    return SubtreeSize(root_);
  }

  static std::size_t SubtreeSize(const Node* n) {
    return n ? n->subtree_size : 0;
  }

  // standard BST transplant: replaces u with v in the tree
  void Transplant(Node* u, Node* v) {
    if (u->parent == nullptr) {
      root_ = v;
    } else if (u == u->parent->left) {
      u->parent->left = v;
    } else {
      u->parent->right = v;
    }
    if (v != nullptr) {
      v->parent = u->parent;
    }
  }

  static Node* Minimum(Node* n) {
    while (n->left != nullptr)
      n = n->left;
    return n;
  }

  static Node* Successor(Node* n) {
    if (n->right != nullptr) {
      return Minimum(n->right);
    }
    Node* p = n->parent;
    while (p != nullptr && n == p->right) {
      n = p;
      p = p->parent;
    }
    return p;
  }

  // find k-th (0-based) smallest node
  static Node* Kth(Node* root, std::size_t k) {
    Node* cur = root;
    while (cur != nullptr) {
      std::size_t left_size = SubtreeSize(cur->left);
      if (k < left_size) {
        cur = cur->left;
      } else if (k == left_size) {
        return cur;
      } else {
        k -= left_size + 1;
        cur = cur->right;
      }
    }
    return nullptr;
  }

  // recursively free nodes
  static void Clear(Node* n) {
    if (n == nullptr)
      return;
    Clear(n->left);
    Clear(n->right);
    delete n;
  }
};

}  // namespace youndex::express
