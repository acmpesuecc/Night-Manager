#include <gtk/gtk.h>
#include <dirent.h>
#include <string.h>

GtkWidget *file_list;
char current_dir[256] = "/"; 

void update_file_list() {
    DIR *d;
    struct dirent *dir;
    GtkListStore *list_store;
    GtkTreeIter iter;

    list_store = GTK_LIST_STORE(gtk_tree_view_get_model(GTK_TREE_VIEW(file_list)));
    gtk_list_store_clear(list_store);

    d = opendir(current_dir);
    if (d) {
        while ((dir = readdir(d)) != NULL) {
            gtk_list_store_append(list_store, &iter);
            gtk_list_store_set(list_store, &iter, 0, dir->d_name, -1);
        }
        closedir(d);
    }
}

void on_item_activated(GtkTreeView *tree_view, GtkTreePath *path, GtkTreeViewColumn *col, gpointer user_data) {
    GtkTreeModel *model;
    GtkTreeIter iter;
    char *selected_file;

    model = gtk_tree_view_get_model(tree_view);
    if (gtk_tree_model_get_iter(model, &iter, path)) {
        gtk_tree_model_get(model, &iter, 0, &selected_file, -1);

        char new_path[512];
        snprintf(new_path, sizeof(new_path), "%s/%s", current_dir, selected_file);

        DIR *d = opendir(new_path);
        if (d) {
            closedir(d);
            strcpy(current_dir, new_path);
            update_file_list();  
        } else {
            g_print("Selected file: %s\n", new_path);  
        }

        g_free(selected_file);
    }
}

int main(int argc, char *argv[]) {
    GtkWidget *window;
    GtkWidget *scrolled_window;
    GtkWidget *vbox;  // Declaration of the vbox widget
    GtkListStore *list_store;
    GtkCellRenderer *renderer;
    GtkTreeViewColumn *column;

    gtk_init(&argc, &argv);

    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "File Manager");
    gtk_window_set_default_size(GTK_WINDOW(window), 600, 400);
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_container_add(GTK_CONTAINER(window), vbox);

    scrolled_window = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled_window), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    gtk_box_pack_start(GTK_BOX(vbox), scrolled_window, TRUE, TRUE, 0);

    list_store = gtk_list_store_new(1, G_TYPE_STRING);
    file_list = gtk_tree_view_new_with_model(GTK_TREE_MODEL(list_store));
    renderer = gtk_cell_renderer_text_new();
    column = gtk_tree_view_column_new_with_attributes("Files", renderer, "text", 0, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(file_list), column);
    gtk_container_add(GTK_CONTAINER(scrolled_window), file_list);

    g_signal_connect(file_list, "row-activated", G_CALLBACK(on_item_activated), NULL);

    gtk_widget_show_all(window);

    update_file_list();
    
    gtk_main();

    return 0;
}
