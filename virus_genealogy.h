#ifndef VIRUS_GENEALOGY_H
#define VIRUS_GENEALOGY_H

#include <algorithm>
#include <map>
#include <memory>
#include <vector>

struct VirusNotFound : std::exception {
    char const* what() const throw() {
        return "VirusNotFound";
    }
};

struct VirusAlreadyCreated : std::exception {
    char const* what() const throw() {
        return "VirusAlreadyCreated";
    }
};

struct TriedToRemoveStemVirus : std::exception {
    char const* what() const throw() {
        return "TriedToRemoveStemVirus";
    }
};

template <typename T>
concept VirusClassType = requires(T t) {
    t.get_id();
};

template <VirusClassType Virus>
class VirusGenealogy {
    class Node;
    using id_type = decltype(std::declval<Virus>().get_id());
    using map_type = std::map<id_type, std::shared_ptr<Node>>;
    const id_type stem_id;
    map_type known_ids;

public:
    class children_iterator;

    // Tworzy nową genealogię.
    // Tworzy także węzeł wirusa macierzystego o identyfikatorze stem_id.
    VirusGenealogy(const id_type &_stem_id) try : stem_id(_stem_id) {
        try {
            known_ids[stem_id] = std::make_shared<Node>(stem_id);
        }
        catch (...) {
            known_ids.erase(stem_id);
            throw;
        }
    }
    catch (...) {
        throw;
    }

    VirusGenealogy(const VirusGenealogy&) noexcept = delete;
    VirusGenealogy& operator=(const VirusGenealogy&) noexcept = delete;

    // Zwraca identyfikator wirusa macierzystego.
    id_type get_stem_id() const {
        try {
            return stem_id;
        }
        catch (...) {
            throw;
        }
    }

    // Zwraca iterator pozwalający przeglądać listę wirusów będących
    // bezpośrednimi następnikami wirusa o podanym identyfikatorze.
    // Zgłasza wyjątek VirusNotFound, jeśli dany wirus nie istnieje.
    // Iterator musi spełniać koncept bidirectional_iterator oraz
    // typeid(*v.get_children_begin()) == typeid(const Virus &).
    VirusGenealogy<Virus>::children_iterator get_children_begin(id_type const &id) const {
        if (!exists(id))
            throw VirusNotFound();

        try {
            return known_ids.at(id)->begin();
        }
        catch (...) {
            throw;
        }
    }

    // Iterator wskazujący na element za końcem wyżej wspomnianej listy.
    // Zgłasza wyjątek VirusNotFound, jeśli dany wirus nie istnieje.
    VirusGenealogy<Virus>::children_iterator get_children_end(id_type const &id) const {
        if (!exists(id))
            throw VirusNotFound();

        try {
            return known_ids.at(id)->end();
        }
        catch (...) {
            throw;
        }
    }

    // Zwraca listę identyfikatorów bezpośrednich poprzedników wirusa
    // o podanym identyfikatorze.
    // Zgłasza wyjątek VirusNotFound, jeśli dany wirus nie istnieje.
    std::vector<id_type> get_parents(id_type const &id) const {
        if (!exists(id))
            throw VirusNotFound();

        try {
            return known_ids.at(id)->get_parents();
        }
        catch (...) {
            throw;
        }
    }


    // Sprawdza, czy wirus o podanym identyfikatorze istnieje.
    bool exists(id_type const &id) const {
        try {
            return known_ids.contains(id);
        }
        catch (...) {
            throw;
        }
    }

    // Sprawdza, czy wirusy o podanych identyfikatorach istnieją.
    bool exists(std::vector<id_type> const &parent_ids) const {
        for (const auto &i : parent_ids) {
            if (!exists(i))
                return false;
        }

        return true;
    }

    // Zwraca referencję do obiektu reprezentującego wirus o podanym
    // identyfikatorze.
    // Zgłasza wyjątek VirusNotFound, jeśli żądany wirus nie istnieje.
    Virus& operator[](id_type const &id) const {
        if (!exists(id))
            throw VirusNotFound();

        try {
            return known_ids.at(id)->get_virus();
        }
        catch (...) {
            throw;
        }
    }

    // Tworzy węzeł reprezentujący nowy wirus o identyfikatorze id
    // powstały z wirusów o podanym identyfikatorze parent_id lub
    // podanych identyfikatorach parent_ids.
    // Zgłasza wyjątek VirusAlreadyCreated, jeśli wirus o identyfikatorze
    // id już istnieje.
    // Zgłasza wyjątek VirusNotFound, jeśli któryś z wyspecyfikowanych
    // poprzedników nie istnieje.
    void create(id_type const &id, id_type const &parent_id) {
        if(exists(id))
            throw VirusAlreadyCreated();

        if(!exists(parent_id))
            throw VirusNotFound();

        try {
            known_ids[id] = std::make_shared<Node>(id);
            known_ids[id]->add_parent(known_ids.at(parent_id));
        }
        catch(...) {
            known_ids.erase(id);
            throw;
        }
    }

    void create(id_type const &id, std::vector<id_type> const &parent_ids) {
        if (exists(id))
            throw VirusAlreadyCreated();

        if (!exists(parent_ids))
            throw VirusNotFound();

        if (parent_ids.empty())
            return;

        try {
            known_ids[id] = std::make_shared<Node>(id);

            for (const auto &parent_id : parent_ids)
                known_ids[id]->add_parent(known_ids.at(parent_id));
        }
        catch (...) {
            known_ids.erase(id);
            throw;
        }
    }

    // Dodaje nową krawędź w grafie genealogii.
    // Zgłasza wyjątek VirusNotFound, jeśli któryś z podanych wirusów nie istnieje.
    void connect(id_type const &child_id, id_type const &parent_id) {
        if (!exists(child_id) || !exists(parent_id))
            throw VirusNotFound();

        try {
            std::vector<id_type> parents = get_parents(child_id);

            for (const auto& p : parents)
                if (p == parent_id) return;
        }
        catch (...) {
            throw;
        }

        known_ids[child_id]->add_parent(known_ids.at(parent_id));
    }

    // Usuwa wirus o podanym identyfikatorze.
    // Zgłasza wyjątek VirusNotFound, jeśli żądany wirus nie istnieje.
    // Zgłasza wyjątek TriedToRemoveStemVirus przy próbie usunięcia
    // wirusa macierzystego.
    void remove(id_type const &id);

    private:
    class Node : public std::enable_shared_from_this<Node> {
        id_type id;
        std::shared_ptr<Virus> virus;
        std::vector<std::weak_ptr<Node>> parents;
        std::vector<std::shared_ptr<Node>> children;

        bool is_orphan() {
            return parents.empty();
        }

        void remove_parent(const id_type &parent_id) {
            auto has_this_id = [parent_id](std::weak_ptr<Node> const &node_ptr) {
                auto parent = node_ptr.lock();
                if (!parent)
                    throw;
                return parent->get_id() == parent_id;
            };

            auto parent = std::ranges::find_if(parents.begin(),parents.end(), has_this_id);
            parents.erase(parent);
        }

        void remove_child(const id_type &child_id) {
            auto has_this_id = [child_id](std::shared_ptr<Node> const &node_ptr) noexcept { return node_ptr->get_id() == child_id; };
            auto child = std::ranges::find_if(children.begin(),children.end(), has_this_id);
            children.erase(child);
        }

    public:
        Node (const id_type _id) try : id(_id) {
            try {
                virus = std::make_shared<Virus>(id);
            }
            catch (...) {
                throw;
            }
        }
        catch (...) {
            throw;
        }

        std::vector<id_type> get_parents() const {
            std::vector<id_type> parents_ids;

            for (const auto &iter : parents) {
                auto parent = iter.lock();
                if (!parent)
                    throw;

                parents_ids.push_back(parent->get_id());
            }

            return parents_ids;
        }

        //Usuwa wszystkie ślady wierzchołka w grafie
        //Zwraca listę id wierzchołków, które po usunięciu,
        //tego wierzchołka nie mają rodzica.
        std::vector<id_type> remove_my_trace() {
            std::vector<id_type> new_orphans;

            for (auto &child : children) {
                child->remove_parent(id);

                if (child->is_orphan())
                    new_orphans.push_back(child->get_id());
            }

            for (auto &iter : parents) {
                auto parent = iter.lock();

                if (!parent)
                    throw;

                parent->remove_child(id);
            }

            return new_orphans;
        }

        void add_parent(const std::shared_ptr<Node> &parent) noexcept {
            parents.push_back(parent);
            parent->children.push_back(this->shared_from_this());
        }

        void add_child(const std::shared_ptr<Node> &child) noexcept {
            children.push_back(child);
            child->parents.push_back(this->shared_from_this());
        }

        id_type get_id() const {
            try {
                return id;
            }
            catch(...) {
                throw;
            }
        }

        Virus& get_virus() const noexcept {
            return *virus;
        }

        children_iterator begin() noexcept{
            return children_iterator(&children[0]);
        }

        children_iterator end() noexcept{
            return children_iterator(&children[children.size()]);
        }
    };
};

// Zdecydowaliśmy się umieścić implementację funkcji poza klasą,
// aby kompilator nie uznawał jej za funkcję inline.
template<VirusClassType Virus>
void VirusGenealogy<Virus>::remove(id_type const &id) {
    if (id == stem_id)
        throw TriedToRemoveStemVirus();

    if (!exists(id))
        throw VirusNotFound();

    try {
        std::vector<id_type> new_orphans = known_ids.at(id)->remove_my_trace();

        for (auto &orphan : new_orphans)
            remove(orphan);

        known_ids.erase(id);
    }
    catch (...) {
        throw;
    }
}

template<VirusClassType Virus>
class VirusGenealogy<Virus>::children_iterator {
    std::shared_ptr<Node> *m_ptr;
public:
    using difference_type   = std::ptrdiff_t;
    using value_type = Virus;
    using pointer = Virus*;
    using reference = const Virus&;
    using iterator_category = std::bidirectional_iterator_tag;

    children_iterator(std::shared_ptr<Node> *ptr) noexcept : m_ptr(ptr) {};

    children_iterator() noexcept {};

    children_iterator(const children_iterator &ch) noexcept = default;

    reference operator*() const {
        return (*m_ptr)->get_virus();
    }

    pointer operator->() const {
        return &(*m_ptr)->get_virus();
    }

    children_iterator& operator++() noexcept {
        m_ptr++;
        return *this;
    }

    children_iterator operator++(int) noexcept {
        children_iterator tmp = *this;
        m_ptr++;
        return tmp;
    }

    children_iterator& operator--() noexcept {
        m_ptr--;
        return *this;
    }

    children_iterator operator--(int) noexcept {
        children_iterator tmp = *this;
        m_ptr--;
        return tmp;
    }

    friend bool operator==(const children_iterator& a, const children_iterator& b) noexcept {
        return a.m_ptr == b.m_ptr;
    };
};

#endif /* VIRUS_GENEALOGY_H */
