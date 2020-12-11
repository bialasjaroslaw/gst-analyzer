#include <gst/gst.h>

#include <map>
#include <set>
#include <string>

#define colored_output 1
#define BLUE "\033[34m"
#define BRBLUE "\033[94m"
#define BRCYAN "\033[96m"
#define BRMAGENTA "\033[95m"
#define BRYELLOW "\033[33m"
#define CYAN "\033[36m"
#define GREEN "\033[32m"
#define MAGENTA "\033[35m"
#define YELLOW "\033[33m"

#define RESET_COLOR (colored_output ? "\033[0m" : "")
#define PLUGIN_NAME_COLOR (colored_output ? BRBLUE : "")
#define ELEMENT_NAME_COLOR (colored_output ? GREEN : "")
#define ELEMENT_DETAIL_COLOR (colored_output ? RESET_COLOR : "")

static std::map<std::string, std::string> name_of_plugin;
static std::map<std::string, std::string> plugin_path;
static GstRegistry* reg = nullptr;
static bool verbose = false;

void print_plugin(const std::string& plugin)
{
    g_print("%s%s%s", PLUGIN_NAME_COLOR, plugin.c_str(), RESET_COLOR);
}

void print_elem(const std::string& elem)
{
    g_print("%s%s%s", ELEMENT_NAME_COLOR, elem.c_str(), RESET_COLOR);
}


void print_plugin_elem(const std::string& plugin, const std::string& elem)
{
    print_plugin(plugin);
    g_print(": ");
    print_elem(elem);
    g_print("\n");
}

void iterate_elements_in_plugin(GstPlugin* plugin)
{
    if (plugin == nullptr)
    {
        if(verbose)
            g_print("Plugin is nullptr\n");
        return;
    }
    const gchar* filename = gst_plugin_get_filename(plugin);
    const gchar* name_plugin = gst_plugin_get_name(plugin);
    if (filename == nullptr || name_plugin == nullptr)
    {
        if (name_plugin == nullptr || strcmp(name_plugin, "staticelements"))
            if(verbose)
                g_print("Can not find info about plugin\n");
        return;
    }
    plugin_path.insert({name_plugin, filename});
    GList* feature_list = gst_registry_get_feature_list_by_plugin(reg, name_plugin);
    GList* features = feature_list;

    if(verbose)
        g_print("Listing %s%s%s elements:\n", PLUGIN_NAME_COLOR, name_plugin, RESET_COLOR);
    while (features)
    {
        do
        {
            if (features->data == nullptr)
                break;

            GstPluginFeature* feature = GST_PLUGIN_FEATURE(features->data);
            if (GST_IS_ELEMENT_FACTORY(feature))
            {
                const gchar* elem_name = GST_OBJECT_NAME(GST_ELEMENT_FACTORY(feature));
                name_of_plugin.insert({elem_name, name_plugin});
                if(verbose)
                    g_print("%s\n", GST_OBJECT_NAME(GST_ELEMENT_FACTORY(feature)));
            }

        } while (false);
        features = g_list_next(features);
    }
    gst_plugin_feature_list_free(feature_list);
}

int main(int argc, char* argv[])
{
    bool compare_element_names = true;
    bool compare_plugin_names = false;
    int start_with = 1;

    for (int idx = 1; idx < argc; ++idx)
    {
        if (strncmp(argv[idx], "--", 2) == 0)
        {
            if (strcmp(argv[idx], "--elements") == 0)
            {
                compare_element_names = true;
                compare_plugin_names = true;
            }
            else if (strcmp(argv[idx], "--plugins") == 0)
            {
                compare_element_names = false;
                compare_plugin_names = true;
            }
            else if (strcmp(argv[idx], "--verbose") == 0)
            {
                verbose = true;
            }
            start_with = idx + 1;
        }
        else
        {
            break;
        }
    }


    gst_init(&argc, &argv);
    reg = gst_registry_get();

    GList* plug_list = gst_registry_get_plugin_list(reg);
    GList* plug_current = plug_list;

    while (plug_current)
    {
        GstPlugin* plugin = GST_PLUGIN(plug_current->data);
        iterate_elements_in_plugin(plugin);
        plug_current = g_list_next(plug_current);
        if (plugin)
            gst_object_unref(GST_OBJECT(plugin));
    }
    gst_plugin_feature_list_free(plug_list);

    std::set<std::string> paths;

    if (compare_element_names)
    {
        for (int arg_c = start_with; arg_c < argc; ++arg_c)
        {
            if (name_of_plugin.count(argv[arg_c]) == 0)
            {
                if (verbose)
                    g_print("Element %s not found!\n", argv[arg_c]);
            }
            else
            {
                const auto& name = name_of_plugin[argv[arg_c]];
                if (verbose)
                {
                    g_print("Found element in plugin");
                    print_plugin_elem(name, argv[arg_c]);
                    g_print(" in %s\n", plugin_path[name].c_str());
                }
                paths.insert(plugin_path[name]);
            }
        }
    }
    else
    {
        for (int arg_c = start_with; arg_c < argc; ++arg_c)
        {
            if (plugin_path.count(argv[arg_c]) == 0)
            {
                if (verbose)
                    g_print("Plugin %s not found!\n", argv[arg_c]);
            }
            else
            {
                const auto& name = argv[arg_c];
                if (verbose)
                {
                    g_print("Found plugin ");
                    print_plugin(name);
                    g_print(" under path %s\n", plugin_path[name].c_str());
                }
                paths.insert(plugin_path[name]);
            }
        }
    }


    for (const auto& path : paths)
        g_print("%s ", path.c_str());
    if (!paths.empty())
        g_print("\n");

    return 0;
}
