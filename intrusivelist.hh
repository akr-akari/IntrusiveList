#ifndef Z_AKR_INTRUSIVELIST_HH
#define Z_AKR_INTRUSIVELIST_HH

#include <algorithm>
#include <concepts>
#include <cstddef>
#include <iterator>
#include <memory>
#include <type_traits>

namespace akr
{
    template<class T>
    struct IntrusiveNode
    {
        template<class U, bool>
        requires(std::derived_from<U, IntrusiveNode<U>>)
        friend struct IntrusiveList;

        private:
        struct NodeIteratorBase
        {
            protected:
            T* nodePtr {};

            public:
            constexpr NodeIteratorBase() = default;

            constexpr NodeIteratorBase(IntrusiveNode<T>* nodePtr_) noexcept:
                nodePtr { static_cast<T*>(nodePtr_) }
            {
            }

            constexpr NodeIteratorBase(std::nullptr_t) noexcept:
                NodeIteratorBase()
            {
            }

            public:
            constexpr operator bool() const noexcept
            {
                return nodePtr != nullptr;
            }
        };

        template<class U, bool IsConst = false>
        struct NodeIterator: NodeIteratorBase
        {
            public:
            using difference_type   = std::ptrdiff_t;
            using value_type        = T;
            using pointer           = std::conditional_t<IsConst, const T*, T*>;
            using reference         = std::conditional_t<IsConst, const T&, T&>;
            using iterator_category = std::bidirectional_iterator_tag;

            private:
            using NodeIteratorBase::nodePtr;

            public:
            using NodeIteratorBase::NodeIteratorBase;

            public:
            constexpr NodeIterator(const NodeIteratorBase& iter) noexcept:
                NodeIteratorBase(iter)
            {
            }

            public:
            constexpr auto operator->() const noexcept -> pointer
            {
                return  nodePtr;
            }

            constexpr auto operator* () const noexcept -> reference
            {
                return *nodePtr;
            }

            public:
            constexpr auto operator++(   ) noexcept -> U&
            {
                auto&& rhs = *static_cast<U*>(this);

                rhs.Increment();

                return rhs;
            }
            constexpr auto operator++(int) noexcept -> U
            {
                auto&& lhs = *static_cast<U*>(this);

                U tmp(lhs);

                ++lhs;

                return tmp;
            }

            constexpr auto operator--(   ) noexcept -> U&
            {
                auto&& rhs = *static_cast<U*>(this);

                rhs.Decrement();

                return rhs;
            }
            constexpr auto operator--(int) noexcept -> U
            {
                auto&& lhs = *static_cast<U*>(this);

                U tmp(lhs);

                --lhs;

                return tmp;
            }

            public:
            friend constexpr auto operator==(const U& lhs, const U& rhs) noexcept -> bool
            {
                return lhs.nodePtr == rhs.nodePtr;
            }
            friend constexpr auto operator!=(const U& lhs, const U& rhs) noexcept -> bool
            {
                return lhs.nodePtr != rhs.nodePtr;
            }
        };

        private:
        template<class U, bool IsConst = false>
        struct ForwardNodeIteratorBase: NodeIterator<U, IsConst>
        {
            template<class V, bool>
            friend struct NodeIterator;

            private:
            using NodeIterator = NodeIterator<U, IsConst>;

            public:
            using NodeIterator::NodeIterator;

            private:
            constexpr void Increment() noexcept
            {
                *static_cast<U*>(this) = (*this)->next;
            }

            constexpr void Decrement() noexcept
            {
                *static_cast<U*>(this) = (*this)->prev;
            }
        };

        struct ForwardNodeIterator      final: ForwardNodeIteratorBase<ForwardNodeIterator>
        {
            private:
            using ForwardNodeIteratorBase = ForwardNodeIteratorBase<ForwardNodeIterator>;

            public:
            using ForwardNodeIteratorBase::ForwardNodeIteratorBase;
        };

        static_assert(std::bidirectional_iterator<ForwardNodeIterator>);
        static_assert(std::equality_comparable   <ForwardNodeIterator>);

        struct ConstForwardNodeIterator final: ForwardNodeIteratorBase<ConstForwardNodeIterator, true>
        {
            private:
            using ForwardNodeIteratorBase = ForwardNodeIteratorBase<ConstForwardNodeIterator, true>;

            public:
            using ForwardNodeIteratorBase::ForwardNodeIteratorBase;

            public:
            constexpr ConstForwardNodeIterator(const ForwardNodeIterator& iter) noexcept:
                ForwardNodeIteratorBase(iter)
            {
            }
        };

        static_assert(std::bidirectional_iterator<ConstForwardNodeIterator>);
        static_assert(std::equality_comparable   <ConstForwardNodeIterator>);

        private:
        template<class U, bool IsConst = false>
        struct ReverseNodeIteratorBase: NodeIterator<U, IsConst>
        {
            template<class V, bool>
            friend struct NodeIterator;

            private:
            using NodeIterator = NodeIterator<U, IsConst>;

            public:
            using NodeIterator::NodeIterator;

            private:
            constexpr void Increment() noexcept
            {
                *static_cast<U*>(this) = (*this)->prev;
            }

            constexpr void Decrement() noexcept
            {
                *static_cast<U*>(this) = (*this)->next;
            }
        };

        struct ReverseNodeIterator      final: ReverseNodeIteratorBase<ReverseNodeIterator>
        {
            private:
            using ReverseNodeIteratorBase = ReverseNodeIteratorBase<ReverseNodeIterator>;

            public:
            using ReverseNodeIteratorBase::ReverseNodeIteratorBase;

            public:
            constexpr ReverseNodeIterator     (const ForwardNodeIterator& iter) noexcept:
                ReverseNodeIteratorBase(iter)
            {
            }
        };

        static_assert(std::bidirectional_iterator<ReverseNodeIterator>);
        static_assert(std::equality_comparable   <ReverseNodeIterator>);

        struct ConstReverseNodeIterator final: ReverseNodeIteratorBase<ConstReverseNodeIterator, true>
        {
            private:
            using ReverseNodeIteratorBase = ReverseNodeIteratorBase<ConstReverseNodeIterator, true>;

            public:
            using ReverseNodeIteratorBase::ReverseNodeIteratorBase;

            public:
            constexpr ConstReverseNodeIterator(const ForwardNodeIterator& iter) noexcept:
                ReverseNodeIteratorBase(iter)
            {
            }
        };

        static_assert(std::bidirectional_iterator<ConstReverseNodeIterator>);
        static_assert(std::equality_comparable   <ConstReverseNodeIterator>);

        private:
        ForwardNodeIterator prev {};

        ForwardNodeIterator next {};

        public:
        friend auto operator<=>(const IntrusiveNode& lhs, const IntrusiveNode& rhs) = default;

        private:
        constexpr void AttachPrev(ForwardNodeIterator node) noexcept
        {
            prev = node->prev;
            next = node;

            if (node->prev)
            {
                node->prev->next = this;
            }
            node->prev = this;
        }

        constexpr void AttachNext(ForwardNodeIterator node) noexcept
        {
            prev = node;
            next = node->next;

            if (node->next)
            {
                node->next->prev = this;
            }
            node->next = this;
        }

        constexpr void Detach    () noexcept
        {
            if (prev)
            {
                prev->next = next;
            }
            if (next)
            {
                next->prev = prev;
            }

            prev = nullptr;
            next = nullptr;
        }
    };

    template<bool Enable>
    struct RecordLength
    {
    };

    template<>
    struct RecordLength<true>
    {
        private:
        std::size_t length {};

        public:
        constexpr auto GetLength() const noexcept -> std::size_t
        {
            return length;
        }

        protected:
        constexpr void SetToZero() noexcept
        {
            length = 0;
        }

        constexpr void IncLength() noexcept
        {
            length++;
        }

        constexpr void DecLength() noexcept
        {
            length--;
        }
    };

    template<class T, bool Enable = true>
    requires(std::derived_from<T, IntrusiveNode<T>>)
    struct IntrusiveList final: RecordLength<Enable>
    {
        template<class U, bool>
        requires(std::derived_from<U, IntrusiveNode<U>>)
        friend struct IntrusiveList;

        private:
        using IntrusiveNode            = akr::IntrusiveNode<T>;

        using ForwardNodeIterator      = typename IntrusiveNode::ForwardNodeIterator;
        using ConstForwardNodeIterator = typename IntrusiveNode::ConstForwardNodeIterator;

        using ReverseNodeIterator      = typename IntrusiveNode::ReverseNodeIterator;
        using ConstReverseNodeIterator = typename IntrusiveNode::ConstReverseNodeIterator;

        using RecordLength             = akr::RecordLength<Enable>;

        public:
        static constexpr bool HasLength = Enable;

        private:
        ForwardNodeIterator head {};

        ForwardNodeIterator last {};

        public:
        constexpr IntrusiveList() = default;

        public:
        constexpr IntrusiveList  (const IntrusiveList&) = delete;

        constexpr auto operator= (const IntrusiveList&) = delete;

        constexpr IntrusiveList  (IntrusiveList&& other) noexcept:
            RecordLength(static_cast<RecordLength&&>(other)),
            head { other.head },
            last { other.last }
        {
            other.Clear();
        }

        constexpr auto operator= (IntrusiveList&& other) noexcept -> IntrusiveList&
        {
            if (this == &other)
            {
                return *this;
            }

            RecordLength::operator=(static_cast<RecordLength&&>(other));

            head = other.head;
            last = other.last;

            other.Clear();

            return *this;
        }

        public:
        template<class U, bool Enable_>
        constexpr IntrusiveList  (IntrusiveList<U, Enable_>&& other) noexcept
        {
            for (auto&& e : other)
            {
                InsertLast(e, other);
            }
        }

        public:
        template<class U, bool Enable_>
        constexpr auto operator+=(IntrusiveList<U, Enable_>&& rhs) noexcept -> IntrusiveList&
        {
            for (decltype(rhs.begin()) iter = rhs.begin(), prev; iter != rhs.end();)
            {
                prev = iter++;

                InsertLast(prev, rhs);
            }

            return *this;
        }

        public:
        template<class U, bool Enable_>
        friend constexpr auto operator+  (IntrusiveList&& lhs, IntrusiveList<U, Enable_>&& rhs) noexcept
            -> IntrusiveList<std::common_type_t<T, U>, Enable || Enable_>
        {
            IntrusiveList<std::common_type_t<T, U>, Enable || Enable_> tmp;

            for (decltype(lhs.begin()) iter = lhs.begin(), prev; iter != lhs.end();)
            {
                prev = iter++;

                tmp.InsertLast(prev, lhs);
            }

            for (decltype(rhs.begin()) iter = rhs.begin(), prev; iter != rhs.end();)
            {
                prev = iter++;

                tmp.InsertLast(prev, rhs);
            }

            return tmp;
        }

        template<class U, bool Enable_>
        requires(std::equality_comparable_with<T, U>)
        friend constexpr auto operator== (const IntrusiveList& lhs, const IntrusiveList<U, Enable_>& rhs) noexcept
            -> bool
        {
            return std::equal(lhs.begin(), lhs.end(), rhs.begin(), rhs.end());
        }

        template<class U, bool Enable_>
        requires(std::three_way_comparable_with<T, U>)
        friend constexpr auto operator<=>(const IntrusiveList& lhs, const IntrusiveList<U, Enable_>& rhs) noexcept
        {
            return std::lexicographical_compare_three_way(lhs.begin(), lhs.end(), rhs.begin(), rhs.end());
        }

        public:
        constexpr auto begin     () const noexcept -> ConstForwardNodeIterator
        {
            return head;
        }
        constexpr auto end       () const noexcept -> ConstForwardNodeIterator
        {
            return nullptr;
        }

        constexpr auto rbegin    () const noexcept -> ConstReverseNodeIterator
        {
            return last;
        }
        constexpr auto rend      () const noexcept -> ConstReverseNodeIterator
        {
            return nullptr;
        }

        constexpr auto cbegin    () const noexcept -> ConstForwardNodeIterator
        {
            return head;
        }
        constexpr auto cend      () const noexcept -> ConstForwardNodeIterator
        {
            return nullptr;
        }

        constexpr auto crbegin   () const noexcept -> ConstReverseNodeIterator
        {
            return last;
        }
        constexpr auto crend     () const noexcept -> ConstReverseNodeIterator
        {
            return nullptr;
        }

        public:
        constexpr auto begin     ()       noexcept -> ForwardNodeIterator
        {
            return head;
        }
        constexpr auto end       ()       noexcept -> ForwardNodeIterator
        {
            return nullptr;
        }

        constexpr auto rbegin    ()       noexcept -> ReverseNodeIterator
        {
            return last;
        }
        constexpr auto rend      ()       noexcept -> ReverseNodeIterator
        {
            return nullptr;
        }

        public:
        constexpr auto GetHead   () const noexcept -> ForwardNodeIterator
        {
            return head;
        }

        constexpr auto GetLast   () const noexcept -> ForwardNodeIterator
        {
            return last;
        }

        constexpr auto IsEmpty   () const noexcept -> bool
        {
            return !head && !last;
        }

        public:
        constexpr void Clear     () noexcept
        {
            last = head = nullptr;

            if constexpr (Enable)
            {
                RecordLength::SetToZero();
            }
        }

        constexpr auto InsertPrev(ForwardNodeIterator curNode, ForwardNodeIterator newNode) noexcept
            -> ForwardNodeIterator
        {
            newNode->Detach();

            if (IsEmpty())
            {
                head = newNode;
                last = newNode;
            }
            else
            {
                newNode->AttachPrev(curNode);

                if (curNode == head)
                {
                    head = newNode;
                }
            }

            if constexpr (Enable)
            {
                RecordLength::IncLength();
            }

            return newNode;
        }

        constexpr auto InsertNext(ForwardNodeIterator curNode, ForwardNodeIterator newNode) noexcept
            -> ForwardNodeIterator
        {
            newNode->Detach();

            if (IsEmpty())
            {
                head = newNode;
                last = newNode;
            }
            else
            {
                newNode->AttachNext(curNode);

                if (curNode == last)
                {
                    last = newNode;
                }
            }

            if constexpr (Enable)
            {
                RecordLength::IncLength();
            }

            return newNode;
        }

        constexpr auto InsertHead(ForwardNodeIterator newNode) noexcept -> ForwardNodeIterator
        {
            return InsertPrev(head, newNode);
        }

        constexpr auto InsertLast(ForwardNodeIterator newNode) noexcept -> ForwardNodeIterator
        {
            return InsertNext(last, newNode);
        }

        template<class U, bool Enable_>
        constexpr auto InsertPrev(ForwardNodeIterator curNode, ForwardNodeIterator newNode,
                                  IntrusiveList<U, Enable_>& other) noexcept -> ForwardNodeIterator
        {
            other.Remove(newNode);

            return InsertPrev(curNode, newNode);
        }

        template<class U, bool Enable_>
        constexpr auto InsertNext(ForwardNodeIterator curNode, ForwardNodeIterator newNode,
                                  IntrusiveList<U, Enable_>& other) noexcept -> ForwardNodeIterator
        {
            other.Remove(newNode);

            return InsertNext(curNode, newNode);
        }

        template<class U, bool Enable_>
        constexpr auto InsertHead(ForwardNodeIterator newNode,
                                  IntrusiveList<U, Enable_>& other) noexcept -> ForwardNodeIterator
        {
            other.Remove(newNode);

            return InsertHead(newNode);
        }

        template<class U, bool Enable_>
        constexpr auto InsertLast(ForwardNodeIterator newNode,
                                  IntrusiveList<U, Enable_>& other) noexcept -> ForwardNodeIterator
        {
            other.Remove(newNode);

            return InsertLast(newNode);
        }

        constexpr void Remove    (ForwardNodeIterator curNode) noexcept
        {
            if (curNode == head)
            {
                ++head;
            }
            if (curNode == last)
            {
                --last;
            }

            curNode->Detach();

            if constexpr (Enable)
            {
                RecordLength::DecLength();
            }
        }

        constexpr void RemoveHead() noexcept
        {
            Remove(head);
        }

        constexpr void RemoveLast() noexcept
        {
            Remove(last);
        }
    };
}

#ifdef  D_AKR_TEST
#include <vector>

namespace akr::test
{
    AKR_TEST(IntrusiveList,
    {
        struct Test: IntrusiveNode<Test>
        {
            int value {};

            Test(int value_) noexcept:
                value { value_ }
            {
            }

            auto operator<=>(const Test&) const = default;
        };

        IntrusiveList<Test> list;
        std::vector  <Test*> vec;

        assert(list.IsEmpty());
        assert(list.GetLength() == 0);

        auto p = list.GetHead();
        assert(!p);

        auto v1 = Test(1);
        auto v2 = Test(2);
        auto v3 = Test(3);

        list.InsertLast(&v1);
        list.InsertLast(&v2);
        list.InsertLast(&v3);

        assert(list.IsEmpty() == false);
        assert(list.GetLength() == 3);

        assert(list.GetHead() == &v1);
        assert(list.GetLast() == &v3);

        p = list.GetHead();
        vec.push_back(&*p);
        assert(p == &v1);
        ++p;
        vec.push_back(&*p);
        assert(p == &v2);
        ++p;
        vec.push_back(&*p);
        assert(p == &v3);

        assert(std::equal(list.  begin(), list.  end(), vec.  begin(), vec.  end(), [](auto&& e1, auto&& e2)
                          {
                              return e1.value == e2->value;
                          }));
        assert(std::equal(list. rbegin(), list. rend(), vec. rbegin(), vec. rend(), [](auto&& e1, auto&& e2)
                          {
                              return e1.value == e2->value;
                          }));
        assert(std::equal(list. cbegin(), list. cend(), vec. cbegin(), vec. cend(), [](auto&& e1, auto&& e2)
                          {
                              return e1.value == e2->value;
                          }));
        assert(std::equal(list.crbegin(), list.crend(), vec.crbegin(), vec.crend(), [](auto&& e1, auto&& e2)
                          {
                              return e1.value == e2->value;
                          }));

        vec.clear();

        list.Remove(&v2);

        assert(list.IsEmpty() == false);
        assert(list.GetLength() == 2);

        assert(list.GetHead() == &v1);
        assert(list.GetLast() == &v3);

        p = list.GetHead();
        vec.push_back(&*p);
        assert(p == &v1);
        ++p;
        vec.push_back(&*p);
        assert(p == &v3);

        assert(std::equal(list.  begin(), list.  end(), vec.  begin(), vec.  end(), [](auto&& e1, auto&& e2)
                          {
                              return e1.value == e2->value;
                          }));
        assert(std::equal(list. rbegin(), list. rend(), vec. rbegin(), vec. rend(), [](auto&& e1, auto&& e2)
                          {
                              return e1.value == e2->value;
                          }));
        assert(std::equal(list. cbegin(), list. cend(), vec. cbegin(), vec. cend(), [](auto&& e1, auto&& e2)
                          {
                              return e1.value == e2->value;
                          }));
        assert(std::equal(list.crbegin(), list.crend(), vec.crbegin(), vec.crend(), [](auto&& e1, auto&& e2)
                          {
                              return e1.value == e2->value;
                          }));

        vec.clear();

        auto v4 = Test(4);
        auto v5 = Test(5);

        list.InsertPrev(list.GetHead(), &v4);
        list.InsertPrev(list.GetLast(), &v5);

        assert(list.IsEmpty() == false);
        assert(list.GetLength() == 4);

        assert(list.GetHead() == &v4);
        assert(list.GetLast() == &v3);

        p = list.GetHead();
        vec.push_back(&*p);
        assert(p == &v4);
        ++p;
        vec.push_back(&*p);
        assert(p == &v1);
        ++p;
        vec.push_back(&*p);
        assert(p == &v5);
        ++p;
        vec.push_back(&*p);
        assert(p == &v3);

        assert(std::equal(list.  begin(), list.  end(), vec.  begin(), vec.  end(), [](auto&& e1, auto&& e2)
                          {
                              return e1.value == e2->value;
                          }));
        assert(std::equal(list. rbegin(), list. rend(), vec. rbegin(), vec. rend(), [](auto&& e1, auto&& e2)
                          {
                              return e1.value == e2->value;
                          }));
        assert(std::equal(list. cbegin(), list. cend(), vec. cbegin(), vec. cend(), [](auto&& e1, auto&& e2)
                          {
                              return e1.value == e2->value;
                          }));
        assert(std::equal(list.crbegin(), list.crend(), vec.crbegin(), vec.crend(), [](auto&& e1, auto&& e2)
                          {
                              return e1.value == e2->value;
                          }));

        vec.clear();

        list.RemoveHead();
        list.RemoveLast();

        assert(list.IsEmpty() == false);
        assert(list.GetLength() == 2);

        assert(list.GetHead() == &v1);
        assert(list.GetLast() == &v5);

        auto v6 = Test(6);
        auto v7 = Test(7);

        list.InsertNext(list.GetHead(), &v6);
        list.InsertNext(list.GetLast(), &v7);

        assert(list.IsEmpty() == false);
        assert(list.GetLength() == 4);

        assert(list.GetHead() == &v1);
        assert(list.GetLast() == &v7);

        p = list.GetHead();
        vec.push_back(&*p);
        assert(p == &v1);
        ++p;
        vec.push_back(&*p);
        assert(p == &v6);
        ++p;
        vec.push_back(&*p);
        assert(p == &v5);
        ++p;
        vec.push_back(&*p);
        assert(p == &v7);

        assert(std::equal(list.  begin(), list.  end(), vec.  begin(), vec.  end(), [](auto&& e1, auto&& e2)
                          {
                              return e1.value == e2->value;
                          }));
        assert(std::equal(list. rbegin(), list. rend(), vec. rbegin(), vec. rend(), [](auto&& e1, auto&& e2)
                          {
                              return e1.value == e2->value;
                          }));
        assert(std::equal(list. cbegin(), list. cend(), vec. cbegin(), vec. cend(), [](auto&& e1, auto&& e2)
                          {
                              return e1.value == e2->value;
                          }));
        assert(std::equal(list.crbegin(), list.crend(), vec.crbegin(), vec.crend(), [](auto&& e1, auto&& e2)
                          {
                              return e1.value == e2->value;
                          }));

        list.Clear();
        assert(list.IsEmpty());
        assert(list.GetLength() == 0);

        vec.clear();

        auto v8 = Test(8);
        auto v9 = Test(9);
        auto v0 = Test(0);

        list.InsertHead(&v8);
        list.InsertHead(&v9);
        list.InsertHead(&v0);

        assert(list.IsEmpty() == false);
        assert(list.GetLength() == 3);

        assert(list.GetHead() == &v0);
        assert(list.GetLast() == &v8);

        p = list.GetHead();
        vec.push_back(&*p);
        assert(p == &v0);
        ++p;
        vec.push_back(&*p);
        assert(p == &v9);
        ++p;
        vec.push_back(&*p);
        assert(p == &v8);

        assert(std::equal(list.  begin(), list.  end(), vec.  begin(), vec.  end(), [](auto&& e1, auto&& e2)
                          {
                              return e1.value == e2->value;
                          }));
        assert(std::equal(list. rbegin(), list. rend(), vec. rbegin(), vec. rend(), [](auto&& e1, auto&& e2)
                          {
                              return e1.value == e2->value;
                          }));
        assert(std::equal(list. cbegin(), list. cend(), vec. cbegin(), vec. cend(), [](auto&& e1, auto&& e2)
                          {
                              return e1.value == e2->value;
                          }));
        assert(std::equal(list.crbegin(), list.crend(), vec.crbegin(), vec.crend(), [](auto&& e1, auto&& e2)
                          {
                              return e1.value == e2->value;
                          }));

        vec.clear();

        IntrusiveList<Test> list1;
        std::vector  <Test*> vec1;

        IntrusiveList<Test> list2;
        std::vector  <Test*> vec2;

        assert(list1 == list2);

        list1.InsertLast(&v1);
        list1.InsertLast(&v2);
        list1.InsertLast(&v3);
        list1.InsertLast(&v4);
        list1.InsertLast(&v5);

        list2.InsertLast(&v6);
        list2.InsertLast(&v7);
        list2.InsertLast(&v8);
        list2.InsertLast(&v9);
        list2.InsertLast(&v0);

        list1.InsertPrev(&v2, &v9, list2);
        list1.InsertLast(&v8, list2);

        assert(list1.IsEmpty() == false);
        assert(list1.GetLength() == 7);

        assert(list2.IsEmpty() == false);
        assert(list2.GetLength() == 3);

        assert(list1 != list2);
        assert(list1 <  list2);
        assert(list1 <= list2);

        vec1.push_back  (&v1);
        vec1.push_back  (&v9);
        vec1.push_back  (&v2);
        vec1.push_back  (&v3);
        vec1.push_back  (&v4);
        vec1.push_back  (&v5);
        vec1.push_back  (&v8);

        vec2.push_back  (&v6);
        vec2.push_back  (&v7);
        vec2.push_back  (&v0);

        assert(std::equal(list1.  begin(), list1.  end(), vec1.  begin(), vec1.  end(), [](auto&& e1, auto&& e2)
                          {
                              return e1.value == e2->value;
                          }));
        assert(std::equal(list1. rbegin(), list1. rend(), vec1. rbegin(), vec1. rend(), [](auto&& e1, auto&& e2)
                          {
                              return e1.value == e2->value;
                          }));
        assert(std::equal(list1. cbegin(), list1. cend(), vec1. cbegin(), vec1. cend(), [](auto&& e1, auto&& e2)
                          {
                              return e1.value == e2->value;
                          }));
        assert(std::equal(list1.crbegin(), list1.crend(), vec1.crbegin(), vec1.crend(), [](auto&& e1, auto&& e2)
                          {
                              return e1.value == e2->value;
                          }));

        assert(std::equal(list2.  begin(), list2.  end(), vec2.  begin(), vec2.  end(), [](auto&& e1, auto&& e2)
                          {
                              return e1.value == e2->value;
                          }));
        assert(std::equal(list2. rbegin(), list2. rend(), vec2. rbegin(), vec2. rend(), [](auto&& e1, auto&& e2)
                          {
                              return e1.value == e2->value;
                          }));
        assert(std::equal(list2. cbegin(), list2. cend(), vec2. cbegin(), vec2. cend(), [](auto&& e1, auto&& e2)
                          {
                              return e1.value == e2->value;
                          }));
        assert(std::equal(list2.crbegin(), list2.crend(), vec2.crbegin(), vec2.crend(), [](auto&& e1, auto&& e2)
                          {
                              return e1.value == e2->value;
                          }));

        vec1.clear();
        vec2.clear();

        list2.InsertNext(&v7, &v4, list1);
        list2.InsertHead(&v3, list1);

        assert(list1.IsEmpty() == false);
        assert(list1.GetLength() == 5);

        assert(list2.IsEmpty() == false);
        assert(list2.GetLength() == 5);

        assert(list1 != list2);
        assert(list1 <  list2);
        assert(list1 <= list2);

        vec1.push_back  (&v1);
        vec1.push_back  (&v9);
        vec1.push_back  (&v2);
        vec1.push_back  (&v5);
        vec1.push_back  (&v8);

        vec2.push_back  (&v3);
        vec2.push_back  (&v6);
        vec2.push_back  (&v7);
        vec2.push_back  (&v4);
        vec2.push_back  (&v0);

        assert(std::equal(list1.  begin(), list1.  end(), vec1.  begin(), vec1.  end(), [](auto&& e1, auto&& e2)
                          {
                              return e1.value == e2->value;
                          }));
        assert(std::equal(list1. rbegin(), list1. rend(), vec1. rbegin(), vec1. rend(), [](auto&& e1, auto&& e2)
                          {
                              return e1.value == e2->value;
                          }));
        assert(std::equal(list1. cbegin(), list1. cend(), vec1. cbegin(), vec1. cend(), [](auto&& e1, auto&& e2)
                          {
                              return e1.value == e2->value;
                          }));
        assert(std::equal(list1.crbegin(), list1.crend(), vec1.crbegin(), vec1.crend(), [](auto&& e1, auto&& e2)
                          {
                              return e1.value == e2->value;
                          }));

        assert(std::equal(list2.  begin(), list2.  end(), vec2.  begin(), vec2.  end(), [](auto&& e1, auto&& e2)
                          {
                              return e1.value == e2->value;
                          }));
        assert(std::equal(list2. rbegin(), list2. rend(), vec2. rbegin(), vec2. rend(), [](auto&& e1, auto&& e2)
                          {
                              return e1.value == e2->value;
                          }));
        assert(std::equal(list2. cbegin(), list2. cend(), vec2. cbegin(), vec2. cend(), [](auto&& e1, auto&& e2)
                          {
                              return e1.value == e2->value;
                          }));
        assert(std::equal(list2.crbegin(), list2.crend(), vec2.crbegin(), vec2.crend(), [](auto&& e1, auto&& e2)
                          {
                              return e1.value == e2->value;
                          }));

        auto list3 = std::move(list1);

        assert(list1.IsEmpty());
        assert(list1.GetLength() == 0);
        assert(list3.IsEmpty() == false);
        assert(list3.GetLength() == 5);

        assert(std::equal(list3.  begin(), list3.  end(), vec1.  begin(), vec1.  end(), [](auto&& e1, auto&& e2)
                          {
                              return e1.value == e2->value;
                          }));
        assert(std::equal(list3. rbegin(), list3. rend(), vec1. rbegin(), vec1. rend(), [](auto&& e1, auto&& e2)
                          {
                              return e1.value == e2->value;
                          }));
        assert(std::equal(list3. cbegin(), list3. cend(), vec1. cbegin(), vec1. cend(), [](auto&& e1, auto&& e2)
                          {
                              return e1.value == e2->value;
                          }));
        assert(std::equal(list3.crbegin(), list3.crend(), vec1.crbegin(), vec1.crend(), [](auto&& e1, auto&& e2)
                          {
                              return e1.value == e2->value;
                          }));

        IntrusiveList<Test> list4;
        list4 = std::move(list2);

        assert(list2.IsEmpty());
        assert(list2.GetLength() == 0);
        assert(list4.IsEmpty() == false);
        assert(list4.GetLength() == 5);

        assert(std::equal(list4.  begin(), list4.  end(), vec2.  begin(), vec2.  end(), [](auto&& e1, auto&& e2)
                          {
                              return e1.value == e2->value;
                          }));
        assert(std::equal(list4. rbegin(), list4. rend(), vec2. rbegin(), vec2. rend(), [](auto&& e1, auto&& e2)
                          {
                              return e1.value == e2->value;
                          }));
        assert(std::equal(list4. cbegin(), list4. cend(), vec2. cbegin(), vec2. cend(), [](auto&& e1, auto&& e2)
                          {
                              return e1.value == e2->value;
                          }));
        assert(std::equal(list4.crbegin(), list4.crend(), vec2.crbegin(), vec2.crend(), [](auto&& e1, auto&& e2)
                          {
                              return e1.value == e2->value;
                          }));

        assert(list3 != list4);
        assert(list3 <  list4);
        assert(list3 <= list4);

        IntrusiveList<Test> list5;
        list5 += std::move(list3);

        assert(list3.IsEmpty());
        assert(list3.GetLength() == 0);
        assert(list5.IsEmpty() == false);
        assert(list5.GetLength() == 5);

        assert(std::equal(list5.  begin(), list5.  end(), vec1.  begin(), vec1.  end(), [](auto&& e1, auto&& e2)
                          {
                              return e1.value == e2->value;
                          }));
        assert(std::equal(list5. rbegin(), list5. rend(), vec1. rbegin(), vec1. rend(), [](auto&& e1, auto&& e2)
                          {
                              return e1.value == e2->value;
                          }));
        assert(std::equal(list5. cbegin(), list5. cend(), vec1. cbegin(), vec1. cend(), [](auto&& e1, auto&& e2)
                          {
                              return e1.value == e2->value;
                          }));
        assert(std::equal(list5.crbegin(), list5.crend(), vec1.crbegin(), vec1.crend(), [](auto&& e1, auto&& e2)
                          {
                              return e1.value == e2->value;
                          }));

        auto list6 = std::move(list5) + std::move(list4);

        assert(list4.IsEmpty());
        assert(list4.GetLength() == 0);
        assert(list6.IsEmpty() == false);
        assert(list6.GetLength() == 10);

        vec1.insert(vec1.end(), vec2.begin(), vec2.end());

        assert(std::equal(list6.  begin(), list6.  end(), vec1.  begin(), vec1.  end(), [](auto&& e1, auto&& e2)
                          {
                              return e1.value == e2->value;
                          }));
        assert(std::equal(list6. rbegin(), list6. rend(), vec1. rbegin(), vec1. rend(), [](auto&& e1, auto&& e2)
                          {
                              return e1.value == e2->value;
                          }));
        assert(std::equal(list6. cbegin(), list6. cend(), vec1. cbegin(), vec1. cend(), [](auto&& e1, auto&& e2)
                          {
                              return e1.value == e2->value;
                          }));
        assert(std::equal(list6.crbegin(), list6.crend(), vec1.crbegin(), vec1.crend(), [](auto&& e1, auto&& e2)
                          {
                              return e1.value == e2->value;
                          }));

        list5.Clear();
        assert(list5.IsEmpty());
        assert(list5.GetLength() == 0);

        list6.Clear();
        assert(list6.IsEmpty());
        assert(list6.GetLength() == 0);

        vec1.clear();
        vec2.clear();
    })
}
#endif//D_AKR_TEST

#endif//Z_AKR_INTRUSIVELIST_HH
