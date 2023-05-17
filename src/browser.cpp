#include <gtk/gtk.h>
#include <webkit2/webkit2.h>

static GtkWidget *back_button;
static GtkWidget *forward_button;
static GtkWidget *refresh_button;
static GtkWidget *search_entry;
static GtkWidget *search_engine_combo;
static GtkWidget *address_entry;
static WebKitWebView *web_view;

typedef enum {
    SEARCH_ENGINE_DUCKDUCKGO,
    SEARCH_ENGINE_GOOGLE,
    SEARCH_ENGINE_BRAVE
} SearchEngine;

static void go_back(GtkButton *button, gpointer user_data) {
    WebKitWebView *web_view = WEBKIT_WEB_VIEW(user_data);
    webkit_web_view_go_back(web_view);
}

static void go_forward(GtkButton *button, gpointer user_data) {
    WebKitWebView *web_view = WEBKIT_WEB_VIEW(user_data);
    webkit_web_view_go_forward(web_view);
}

static void refresh(GtkButton *button, gpointer user_data) {
    webkit_web_view_reload(web_view);
}

static gboolean refresh_shortcut(GtkWidget *widget, GdkEventKey *event, gpointer user_data)
{
    if ((event->state & GDK_CONTROL_MASK) && (event->keyval == GDK_KEY_r || event->keyval == GDK_KEY_R))
    {
        WebKitWebView *web_view = WEBKIT_WEB_VIEW(user_data);
        webkit_web_view_reload(web_view);
        return TRUE;
    }
    return FALSE;
}

static void search(GtkEntry *entry, gpointer data) {
    const char *engine = gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(search_engine_combo));
    const char *query = gtk_entry_get_text(entry);
    // Search on the selected search engine
    if (g_strcmp0(engine, "DuckDuckGo") == 0) {
        char *url = g_strdup_printf("https://duckduckgo.com/?q=%s", query);
        webkit_web_view_load_uri(web_view, url);
        g_free(url);
    } else if (g_strcmp0(engine, "Google") == 0) {
        char *url = g_strdup_printf("https://www.google.com/search?q=%s", query);
        webkit_web_view_load_uri(web_view, url);
        g_free(url);
    } else if (g_strcmp0(engine, "Brave") == 0) {
        char *url = g_strdup_printf("https://search.brave.com/search?q=%s", query);
        webkit_web_view_load_uri(web_view, url);
        g_free(url);
    }
}
static void goto_url(GtkEntry* entry, gpointer data) {
    // Open a URL
    const char* query = gtk_entry_get_text(GTK_ENTRY(entry));
    if (!g_str_has_prefix(query, "http://") && !g_str_has_prefix(query, "https://")) {
        gchar* uri = g_strdup_printf("https://%s", query);
        webkit_web_view_load_uri(web_view, uri);
        g_free(uri);
    } else {
        webkit_web_view_load_uri(web_view, query);
    }
}

void on_load_changed(WebKitWebView* web_view, WebKitLoadEvent load_event, gpointer data) {
    GtkWidget* address_entry = static_cast<GtkWidget*>(data);
    if (load_event == WEBKIT_LOAD_STARTED) {
        const gchar* uri = webkit_web_view_get_uri(web_view);
        gtk_entry_set_text(GTK_ENTRY(address_entry), uri);
    }
}

static GtkWidget* create_window() {
    GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "Aqua Browser");
    gtk_window_set_default_size(GTK_WINDOW(window), 1200, 800);

    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_container_add(GTK_CONTAINER(window), vbox);

    // Toolbar (back, forward, refresh, search bar, search engine combobox)
    GtkWidget *toolbar = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_box_pack_start(GTK_BOX(vbox), toolbar, FALSE, FALSE, 0);

    back_button = gtk_button_new_from_icon_name("go-previous", GTK_ICON_SIZE_BUTTON);
    gtk_box_pack_start(GTK_BOX(toolbar), back_button, FALSE, FALSE, 0);

    forward_button = gtk_button_new_from_icon_name("go-next", GTK_ICON_SIZE_BUTTON);
    gtk_box_pack_start(GTK_BOX(toolbar), forward_button, FALSE, FALSE, 0);

    refresh_button = gtk_button_new_from_icon_name("view-refresh", GTK_ICON_SIZE_BUTTON);
    gtk_box_pack_start(GTK_BOX(toolbar), refresh_button, FALSE, FALSE, 0);

    search_entry = gtk_search_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(search_entry), "Enter your search query");
    gtk_box_pack_start(GTK_BOX(toolbar), search_entry, TRUE, TRUE, 0);

    search_engine_combo = gtk_combo_box_text_new();
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(search_engine_combo), "DuckDuckGo");
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(search_engine_combo), "Google");
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(search_engine_combo), "Brave");
    gtk_combo_box_set_active(GTK_COMBO_BOX(search_engine_combo), 0);
    gtk_box_pack_start(GTK_BOX(toolbar), search_engine_combo, FALSE, FALSE, 0);

    toolbar = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_box_pack_start(GTK_BOX(vbox), toolbar, FALSE, FALSE, 0);

    web_view = WEBKIT_WEB_VIEW(webkit_web_view_new());
    gtk_widget_set_size_request(GTK_WIDGET(web_view), -1, -1);
    gtk_box_pack_start(GTK_BOX(vbox), GTK_WIDGET(web_view), TRUE, TRUE, 0);

    // Address Bar (+ tabs, as combobox, in future)
    GtkWidget *address_bar = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_box_pack_start(GTK_BOX(vbox), address_bar, FALSE, FALSE, 0);

    address_entry = gtk_entry_new();
    gtk_entry_set_icon_from_icon_name(GTK_ENTRY(address_entry), GTK_ENTRY_ICON_PRIMARY, "globe");
    gtk_entry_set_placeholder_text(GTK_ENTRY(address_entry), "Enter website address");
    gtk_box_pack_start(GTK_BOX(address_bar), address_entry, TRUE, TRUE, 0);

    gtk_window_set_icon_from_file(GTK_WINDOW(window), "src/img/logo.png", NULL);

    const gchar *uri = "https://andus.dev/aqua";
    webkit_web_view_load_uri(web_view, uri);

    return window;
}

int main(int argc, char *argv[]) {
    gtk_init(&argc, &argv);

    GtkWidget *window = create_window();

    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);
    g_signal_connect(search_entry, "activate", G_CALLBACK(search), web_view);
    g_signal_connect(back_button, "clicked", G_CALLBACK(go_back), web_view);
    g_signal_connect(forward_button, "clicked", G_CALLBACK(go_forward), web_view);
    g_signal_connect(refresh_button, "clicked", G_CALLBACK(refresh), NULL);
    g_signal_connect(address_entry, "activate", G_CALLBACK(goto_url), web_view);
    g_signal_connect(web_view, "load-changed", G_CALLBACK(on_load_changed), address_entry);

    g_signal_connect(window, "key-press-event", G_CALLBACK(refresh_shortcut), web_view);

    gtk_widget_show_all(window);

    gtk_main();

    return 0;
}