#include "../std_include.hpp"
#include "component_loader.hpp"

#include <algorithm>

namespace component_loader
{
    namespace
    {
        using component_entry = std::unique_ptr<component_interface>;
        using component_vector = std::vector<component_entry>;
        using component_vector_destructor = std::function<void(component_vector*)>;
        using component_vector_container = std::unique_ptr<component_vector, component_vector_destructor>;

        component_vector& get_components()
        {
            static component_vector_container components(new component_vector, [](const component_vector* component_vector) {
                pre_destroy();
                delete component_vector;
            });

            return *components;
        }
    }

    void register_component(std::unique_ptr<component_interface>&& component)
    {
        auto& components = get_components();
        components.emplace_back(std::move(component));

        std::ranges::stable_sort(components,
                                 [](const component_entry& a, const component_entry& b) { return a->priority() > b->priority(); });
    }

    bool post_start()
    {
        static auto handled = false;
        if (handled)
            return true;
        handled = true;

        try
        {
            for (const auto& component : get_components())
            {
                component->post_start();
            }
        }
        catch (premature_shutdown_trigger&)
        {
            return false;
        }

        return true;
    }

    bool post_load()
    {
        static auto res = [] {
            try
            {
                for (const auto& component : get_components())
                {
                    component->post_load();
                }
            }
            catch (premature_shutdown_trigger&)
            {
                return false;
            }
            catch (const std::exception& e)
            {
                MessageBoxA(nullptr, e.what(), "Error", MB_ICONERROR | MB_SETFOREGROUND | MB_TOPMOST);
                return false;
            }

            return true;
        }();

        return res;
    }

    void pre_destroy()
    {
        static auto res = [] {
            try
            {
                for (const auto& component : get_components())
                {
                    component->pre_destroy();
                }
            }
            catch (const std::exception& e)
            {
                MessageBoxA(nullptr, e.what(), "Error", MB_ICONERROR | MB_SETFOREGROUND | MB_TOPMOST);
                return false;
            }

            return true;
        }();

        if (!res)
        {
            TerminateProcess(GetCurrentProcess(), 1);
        }
    }

    void* load_import(const std::string& module, const std::string& function)
    {
        void* function_ptr = nullptr;

        for (const auto& component : get_components())
        {
            const auto module_function_ptr = component->load_import(module, function);
            if (module_function_ptr)
            {
                function_ptr = module_function_ptr;
            }
        }

        return function_ptr;
    }

    void trigger_premature_shutdown()
    {
        throw premature_shutdown_trigger();
    }
}
