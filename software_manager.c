#include <gtk/gtk.h>
#include <vte/vte.h>
#include <stdlib.h>
#include <stdio.h>

GtkWidget *package_list;
GtkWidget *details_label;
GtkWidget *terminal;
GtkWidget *toggle_terminal_button;
gboolean terminal_visible = FALSE;

void fetch_available_packages(GtkListStore *list_store);
void on_package_selected(GtkTreeView *tree_view, GtkTreePath *path, GtkTreeViewColumn *col, gpointer user_data);
void on_install_clicked(GtkWidget *widget, gpointer data);
void on_uninstall_clicked(GtkWidget *widget, gpointer data);
void on_toggle_terminal_clicked(GtkWidget *widget, gpointer data);

void fetch_available_packages(GtkListStore *list_store) {
    FILE *fp;
    char output[1024];
    
    fp = popen("pacman -Slq", "r");
    if (fp == NULL) {
        perror("Failed to run pacman command");
        return;
    }

    GtkTreeIter iter;
    while (fgets(output, sizeof(output), fp) != NULL) {
        output[strcspn(output, "\n")] = 0;
        gtk_list_store_append(list_store, &iter);
        gtk_list_store_set(list_store, &iter, 0, output, -1);
    }
    pclose(fp);
}

void on_package_selected(GtkTreeView *tree_view, GtkTreePath *path, GtkTreeViewColumn *col, gpointer user_data) {
    GtkTreeModel *model;
    GtkTreeIter iter;
    char *selected_package;

    model = gtk_tree_view_get_model(tree_view);
    if (gtk_tree_model_get_iter(model, &iter, path)) {
        gtk_tree_model_get(model, &iter, 0, &selected_package, -1);
        
        gchar *details = g_strdup_printf("Package: %s\nVersion: 1.0.0\nSize: 10MB", selected_package);
        gtk_label_set_text(GTK_LABEL(details_label), details);
        g_free(details);
        g_free(selected_package);
    }
}

// Install button callback
void on_install_clicked(GtkWidget *widget, gpointer data) {
    g_print("Installing package...\n");
    
    // The command that will be executed when the package is installed
    const char *command[] = { "/bin/sh", "-c", "echo 'Installing package...' && sudo pacman -S --noconfirm <package-name>", NULL };
    
    // Spawn the command in the embedded terminal
    vte_terminal_spawn_async(
        VTE_TERMINAL(terminal),       // The VTE terminal widget
        VTE_PTY_DEFAULT,              // Use the default pseudo-terminal
        NULL,                         // Working directory (NULL uses current directory)
        (char **)command,             // Command to execute
        NULL,                         // Environment (NULL uses current environment)
        G_SPAWN_DEFAULT,              // Spawn flags
        NULL,                         // Child setup function (not needed here)
        NULL,                         // User data for the setup function (not needed)
        NULL,                         // Cancellable object (not needed)
        -1,                           // Timeout for the operation (-1 means no timeout)
        NULL,                         // GCancellable (not needed)
        NULL,                         // Callback for process completion (not needed)
        NULL                          // User data for the callback (not needed)
    );
}

// Uninstall button callback
void on_uninstall_clicked(GtkWidget *widget, gpointer data) {
    g_print("Uninstalling package...\n");
    
    // Prepare the command to be executed in the terminal
    const char *command[] = { "/bin/sh", "-c", "echo 'Uninstalling package...' && sudo pacman -R --noconfirm <package-name>", NULL };
    
    // Spawn the command in the embedded terminal
    vte_terminal_spawn_async(
        VTE_TERMINAL(terminal),       // The VTE terminal widget
        VTE_PTY_DEFAULT,              // Use the default pseudo-terminal
        NULL,                         // Working directory (NULL uses current directory)
        (char **)command,             // Command to execute
        NULL,                         // Environment (NULL uses current environment)
        G_SPAWN_DEFAULT,              // Spawn flags
        NULL,                         // Child setup function (not needed here)
        NULL,                         // User data for the setup function (unnecessary)
        NULL,                         // Cancellable object (not needed)
        -1,                           // Timeout for the operation (-1 means no timeout)
        NULL,                         // GCancellable (not needed)
        NULL,                         // Callback for process completion (not needed)
        NULL                          // User data for the callback (not needed)
    );
}

// Toggle terminal visibility
void on_toggle_terminal_clicked(GtkWidget *widget, gpointer data) {
    terminal_visible = !terminal_visible;
    if (terminal_visible) {
        gtk_widget_show(terminal);
        gtk_button_set_label(GTK_BUTTON(toggle_terminal_button), "Hide Terminal");
    } else {
        gtk_widget_hide(terminal);
        gtk_button_set_label(GTK_BUTTON(toggle_terminal_button), "Show Terminal");
    }
}

int main(int argc, char *argv[]) {
    GtkWidget *window, *vbox, *scrolled_window, *button_box, *install_button, *uninstall_button;
    GtkListStore *list_store;
    GtkCellRenderer *renderer;
    GtkTreeViewColumn *column;

    gtk_init(&argc, &argv);

    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "Software Manager");
    gtk_window_set_default_size(GTK_WINDOW(window), 800, 600);
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_container_add(GTK_CONTAINER(window), vbox);

    scrolled_window = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled_window), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    gtk_box_pack_start(GTK_BOX(vbox), scrolled_window, TRUE, TRUE, 0);

    list_store = gtk_list_store_new(1, G_TYPE_STRING);
    fetch_available_packages(list_store);

    package_list = gtk_tree_view_new_with_model(GTK_TREE_MODEL(list_store));
    renderer = gtk_cell_renderer_text_new();
    column = gtk_tree_view_column_new_with_attributes("Available Packages", renderer, "text", 0, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(package_list), column);

    gtk_container_add(GTK_CONTAINER(scrolled_window), package_list);
    g_signal_connect(package_list, "row-activated", G_CALLBACK(on_package_selected), NULL);

    details_label = gtk_label_new("Select a package to view details.");
    gtk_box_pack_start(GTK_BOX(vbox), details_label, FALSE, FALSE, 0);

    button_box = gtk_button_box_new(GTK_ORIENTATION_HORIZONTAL);
    gtk_box_pack_start(GTK_BOX(vbox), button_box, FALSE, FALSE, 0);

    install_button = gtk_button_new_with_label("Install");
    g_signal_connect(install_button, "clicked", G_CALLBACK(on_install_clicked), NULL);
    gtk_container_add(GTK_CONTAINER(button_box), install_button);

    uninstall_button = gtk_button_new_with_label("Uninstall");
    g_signal_connect(uninstall_button, "clicked", G_CALLBACK(on_uninstall_clicked), NULL);
    gtk_container_add(GTK_CONTAINER(button_box), uninstall_button);

    terminal = vte_terminal_new();
    gtk_box_pack_start(GTK_BOX(vbox), terminal, TRUE, TRUE, 0);
    gtk_widget_hide(terminal);

    toggle_terminal_button = gtk_button_new_with_label("Show Terminal");
    g_signal_connect(toggle_terminal_button, "clicked", G_CALLBACK(on_toggle_terminal_clicked), NULL);
    gtk_box_pack_start(GTK_BOX(vbox), toggle_terminal_button, FALSE, FALSE, 0);

    gtk_widget_show_all(window);
    gtk_main();

    return 0;
}
