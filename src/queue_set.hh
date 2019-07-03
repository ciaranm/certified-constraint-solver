/* vim: set sw=4 sts=4 et foldmethod=syntax : */

#ifndef CERTIFIED_CONSTRAINT_SOLVER_GUARD_SRC_QUEUE_SET_HH
#define CERTIFIED_CONSTRAINT_SOLVER_GUARD_SRC_QUEUE_SET_HH 1

template <typename T_>
class QueueSet
{
    private:
        std::set<T_> _values;

    public:
        QueueSet() = default;
        ~QueueSet() = default;

        auto empty() const -> bool
        {
            return _values.empty();
        }

        auto enqueue(T_ t) -> void
        {
            _values.insert(t);
        }

        auto dequeue() -> T_
        {
            auto result = *_values.begin();
            _values.erase(_values.begin());
            return result;
        }
};

#endif
