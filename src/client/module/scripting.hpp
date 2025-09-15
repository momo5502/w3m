#pragma once
#pragma warning(disable : 4324)

namespace scripting
{
    namespace game
    {
        __declspec(align(16)) struct Vector
        {
            float X{0.0};
            float Y{0.0};
            float Z{0.0};
            float W{0.0};
        };

        struct EulerAngles
        {
            float Roll{0.0};
            float Pitch{0.0};
            float Yaw{0.0};
        };

        struct CFunction
        {
            char pad[192];
        };

        struct global_script_context;

        struct script_execution_context
        {
            uint64_t some_value;
            char pad[0x28];
            uint8_t* some_stack;
        };

#pragma pack(push)
#pragma pack(4)
        template <typename T>
        struct raw_array
        {
            T* values{};
            uint32_t length{};
        };
#pragma pack(pop)

        using script_function = void(void* a1, script_execution_context* ctx, void* return_value);
    }

    void* allocate_memory(size_t size);
    void free_memory(void* memory);

    template <typename T>
    class array
    {
      public:
        using element_type = T;

        array() = default;

        array(const size_t count, T element = T())
            : array()
        {
            this->resize(count, std::move(element));
        }

        array(const T* data, const size_t count)
        {
            this->array_ = this->allocate_uninitialized(count);

            for (size_t i = 0; i < count; ++i)
            {
                new (&this->array_.values[i]) T(data[i]);
            }
        }

        array(const std::span<const T> data)
            : array(data.data(), data.size())
        {
        }

        array(std::vector<T>&& data)
        {
            this->array_ = this->allocate_uninitialized(data.size());

            for (size_t i = 0; i < data.size(); ++i)
            {
                new (&this->array_.values[i]) T(std::move(data[i]));
            }
        }

        array(const array& obj)
            : array()
        {
            this->operator=(obj);
        }

        array(array&& obj) noexcept
            : array()
        {
            this->operator=(std::move(obj));
        }

        array& operator=(const array& obj)
        {
            if (this != &obj)
            {
                *this = array(obj.array_.values, obj.array_.length);
            }

            return *this;
        }

        array& operator=(array&& obj) noexcept
        {
            if (this != &obj)
            {
                this->clear();

                this->array_ = obj.array_;

                obj.array_.length = 0;
                obj.array_.values = nullptr;
            }

            return *this;
        }

        array& operator=(const std::span<const T> data)
        {
            *this = array(data);
            return *this;
        }

        array& operator=(std::vector<T>&& data)
        {
            *this = array(std::move(data));
            return *this;
        }

        ~array()
        {
            this->clear();
        }

        T* data()
        {
            return this->array_.values;
        }

        const T* data() const
        {
            return this->array_.values;
        }

        size_t size() const
        {
            return this->array_.length;
        }

        size_t size_in_bytes() const
        {
            return this->size() * sizeof(element_type);
        }

        bool empty() const
        {
            return this->size() == 0;
        }

        T& at(const size_t index)
        {
            if (this->size() <= index)
            {
                throw std::runtime_error("Invalid array index");
            }

            return this->data()[index];
        }

        const T& at(const size_t index) const
        {
            if (this->size() <= index)
            {
                throw std::runtime_error("Invalid array index");
            }

            return this->data()[index];
        }

        T& operator[](const size_t index)
        {
            return this->at(index);
        }

        const T& operator[](const size_t index) const
        {
            return this->at(index);
        }

        void push_back(T obj)
        {
            this->resize(this->size() + 1, std::move(obj));
        }

        T pop_back()
        {
            if (this->empty())
            {
                throw std::runtime_error("Array is empty");
            }

            auto& last = this->at(this->size() - 1);
            auto element = std::move(last);
            last.~T();

            this->array_.length -= 1;

            return element;
        }

        T* begin()
        {
            return this->data();
        }

        const T* begin() const
        {
            return this->data();
        }

        T* end()
        {
            return this->begin() + this->size();
        }

        const T* end() const
        {
            return this->begin() + this->size();
        }

        std::vector<T> to_vector() const
        {
            return {this->begin(), this->end()};
        }

        std::vector<T> move_to_vector()
        {
            std::vector<T> v{};
            v.reserve(this->size());

            for (auto& element : *this)
            {
                v.emplace_back(std::move(element));
            }

            this->clear();

            return v;
        }

        void clear()
        {
            while (!this->empty())
            {
                (void)this->pop_back();
            }

            free_memory(this->array_.values);

            this->array_.length = 0;
            this->array_.values = nullptr;
        }

        void resize(const size_t count, T element = T())
        {
            while (count < this->size())
            {
                (void)this->pop_back();
            }

            if (count <= this->size())
            {
                return;
            }

            const auto old_size = this->size();
            auto new_array = this->allocate_uninitialized(count);

            for (size_t i = 0; i < old_size; ++i)
            {
                new (&new_array.values[i]) T(std::move(this->at(i)));
            }

            for (auto i = old_size + 1; i < count; ++i)
            {
                new (&new_array.values[i]) T(element);
            }

            new (&new_array.values[old_size]) T(std::move(element));

            this->clear();

            this->array_ = new_array;
        }

      private:
        game::raw_array<T> allocate_uninitialized(const size_t count) const
        {
            game::raw_array<T> new_array{};
            new_array.length = static_cast<uint32_t>(count);
            if (new_array.length != count)
            {
                throw std::runtime_error("Too many items");
            }

            new_array.values = static_cast<T*>(allocate_memory(sizeof(T) * count));

            return new_array;
        }

        game::raw_array<T> array_{nullptr, 0};
    };

    class string : /* private */ array<wchar_t>
    {
      public:
        string() = default;

        string(const std::string_view& str);
        string& operator=(const std::string_view& str);

        string(const std::wstring_view& str);
        string& operator=(const std::wstring_view& str);

        string(const wchar_t* str);
        string& operator=(const wchar_t* str);

        string(const char* str, size_t length);

        string(const char* str);
        string& operator=(const char* str);

        bool operator==(const string& obj) const;
        bool operator!=(const string& obj) const;

        bool operator==(const std::string_view& obj) const;
        bool operator!=(const std::string_view& obj) const;

        bool operator==(const std::wstring_view& obj) const;
        bool operator!=(const std::wstring_view& obj) const;

        std::wstring to_wstring() const;
        std::string to_string() const;
        std::wstring_view to_view() const;
    };

    namespace detail
    {
        void read_script_argument(game::script_execution_context& ctx, void* value);
        void register_script_function(const wchar_t* name, game::script_function* function);

        template <typename T>
        T read_script_argument(game::script_execution_context& ctx)
        {
            T value{};
            read_script_argument(ctx, &value);
            return value;
        }

        template <>
        inline std::wstring read_script_argument<std::wstring>(game::script_execution_context& ctx)
        {
            const auto managed_string = read_script_argument<string>(ctx);
            return managed_string.to_wstring();
        }

        template <>
        inline std::string read_script_argument<std::string>(game::script_execution_context& ctx)
        {
            const auto managed_string = read_script_argument<string>(ctx);
            return managed_string.to_string();
        }

        template <typename T>
        auto adapt_return_value(T val)
        {
            return val;
        }

        template <>
        inline auto adapt_return_value(std::string val)
        {
            return string(val);
        }

        template <auto Function, typename Return, typename... Args>
        void dispatcher_function(void* /*a1*/, game::script_execution_context* ctx, void* return_value)
        {
            try
            {
                std::tuple<Args...> args{read_script_argument<Args>(*ctx)...};
                if constexpr (std::is_same_v<Return, void>)
                {
                    std::apply(Function, std::move(args));
                }
                else
                {
                    auto value = adapt_return_value(std::apply(Function, std::move(args)));

                    if (return_value)
                    {
                        *static_cast<decltype(value)*>(return_value) = std::move(value);
                    }
                }
            }
            catch (const std::exception& e)
            {
                OutputDebugStringA(e.what());
            }

            ++ctx->some_stack;
        }

        template <typename Return, typename... Args>
        struct dispatcher_helper
        {
            dispatcher_helper(Return (*)(Args...))
            {
            }

            template <auto Function>
            game::script_function* create_dispatcher() const
            {
                return &dispatcher_function<Function, Return, std::remove_cvref_t<Args>...>;
            }
        };

        template <auto Function>
        game::script_function* create_dispatcher_function()
        {
            const dispatcher_helper helper{Function};
            return helper.template create_dispatcher<Function>();
        }
    }

    template <auto Function>
    void register_function(const wchar_t* name)
    {
        auto* dispatcher = detail::create_dispatcher_function<Function>();
        detail::register_script_function(name, dispatcher);
    }

    int register_name_string(const wchar_t* name);
    void call_game_function(const wchar_t* name);
}
