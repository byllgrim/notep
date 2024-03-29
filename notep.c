/* See LICENSE file for copyright and license details. */

#include <gtk/gtk.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "config.h"

enum enum_unsaved_dialog { YES, NO, CANCEL };
enum enum_svga_resolution { SVGA_WIDTH = 800, SVGA_HEIGHT = 600 };
enum enum_buflen { CSS_STRING_MAXLEN = 1024 };

static GtkBuilder           *builder;
static GtkCssProvider       *css_provider;
static GtkTextBuffer        *buffer;
static GtkWidget            *text_view;
static GtkWidget            *text_window;
static GtkWidget            *window;
static PangoFontDescription *font_description;
static char                 *filename = NULL;
static int                   saved    = 0;

static int
unsaved_dialog ( void ) {
    GtkWidget *dialog;
    gint       res;

    dialog = gtk_message_dialog_new (
        GTK_WINDOW ( window ),
        GTK_DIALOG_MODAL,
        GTK_MESSAGE_QUESTION,
        GTK_BUTTONS_NONE,
        "The document has been modified.\n"
        "Save changes?"
    );
    gtk_dialog_add_buttons (
        GTK_DIALOG ( dialog ), "Yes", YES, "No", NO, "Cancel", CANCEL, NULL
    );

    res = gtk_dialog_run ( GTK_DIALOG ( dialog ) );
    gtk_widget_destroy ( dialog );

    return res;
}

static int
save_file ( void ) {
    FILE        *file;
    char        *text;
    GtkTextIter *start;
    GtkTextIter *end;

    start = malloc ( sizeof ( GtkTextIter ) );
    end   = malloc ( sizeof ( GtkTextIter ) );

    file = fopen ( filename, "w" );
    gtk_text_buffer_get_start_iter ( buffer, start );
    gtk_text_buffer_get_end_iter ( buffer, end );
    text = gtk_text_buffer_get_text ( buffer, start, end, TRUE );
    (void)fprintf ( file, "%s", text );

    saved = 1;
    gtk_text_buffer_set_modified ( buffer, FALSE );
    (void)fclose ( file );
    return YES;
}

GtkWidget *
create_saveas_file_chooser_dialog ( void ) {
    GtkWidget *dialog;

    dialog = gtk_file_chooser_dialog_new (
        "Save file",
        GTK_WINDOW ( window ),
        GTK_FILE_CHOOSER_ACTION_SAVE,
        "_Cancel",
        GTK_RESPONSE_CANCEL,
        "_Save",
        GTK_RESPONSE_ACCEPT,
        NULL
    );

    gtk_file_chooser_set_do_overwrite_confirmation (
        GTK_FILE_CHOOSER ( dialog ), TRUE
    );

    if ( filename ) {
        gtk_file_chooser_set_filename ( GTK_FILE_CHOOSER ( dialog ), filename );
    }

    return dialog;
}

static int
activate_button_saveas ( void ) {
    GtkWidget *dialog;
    gint       res;

    dialog = create_saveas_file_chooser_dialog ();
    res    = gtk_dialog_run ( GTK_DIALOG ( dialog ) );

    if ( res == GTK_RESPONSE_ACCEPT ) {
        filename = gtk_file_chooser_get_filename ( GTK_FILE_CHOOSER ( dialog ) );
        save_file ();
        res = YES;
    } else {
        res = CANCEL;
    }

    gtk_widget_destroy ( dialog );
    return res;
}

static int
activate_button_save ( void ) {
    if ( !filename ) {
        return activate_button_saveas ();
    }

    if ( gtk_text_buffer_get_modified ( buffer ) == FALSE ) {
        return YES;
    }

    return save_file ();
}

static int
can_exit ( void ) {
    int prompt;

    if ( gtk_text_buffer_get_modified ( buffer ) == FALSE ) {
        return 1;
    }

    prompt = unsaved_dialog ();
    if ( prompt == YES ) {
        prompt = activate_button_save ();
    }
    return ( prompt != CANCEL );
}

static gboolean
exit_notep ( void ) {
    if ( can_exit () ) {
        exit ( EXIT_SUCCESS );
    }

    return TRUE;
}

static void
die ( char *msg ) {
    GtkWidget *dialog;

    dialog = gtk_message_dialog_new (
        GTK_WINDOW ( window ),
        GTK_DIALOG_DESTROY_WITH_PARENT,
        GTK_MESSAGE_ERROR,
        GTK_BUTTONS_OK,
        "%s",
        msg
    );
    gtk_dialog_run ( GTK_DIALOG ( dialog ) );
    gtk_widget_destroy ( dialog );

    (void)fprintf ( stderr, "%s\n", msg );
    exit ( EXIT_FAILURE );
}

static void
load_file ( void ) {
    FILE  *file;
    size_t length;
    char  *text;

    file = fopen ( filename, "r" );
    (void)fseek ( file, 0, SEEK_END );
    length = ftell ( file );
    (void)fseek ( file, 0, SEEK_SET );

    text = malloc ( (size_t)( length + 1 ) );
    if ( fread ( text, sizeof ( char ), length, file ) != length ) {
        die ( "notep: error reading file" );
    }
    text[length] = '\0';
    gtk_text_buffer_set_text ( buffer, text, (gint)length );

    saved = 1;
    gtk_text_buffer_set_modified ( buffer, FALSE );
    (void)fclose ( file );
}

static void
run_file_chooser_load_file ( void ) {
    GtkWidget *file_chooser;
    gint       res;

    file_chooser = gtk_file_chooser_dialog_new (
        "Open file",
        GTK_WINDOW ( window ),
        GTK_FILE_CHOOSER_ACTION_OPEN,
        "_Cancel",
        GTK_RESPONSE_CANCEL,
        "_Open",
        GTK_RESPONSE_ACCEPT,
        NULL
    );
    res = gtk_dialog_run ( GTK_DIALOG ( file_chooser ) );

    if ( res == GTK_RESPONSE_ACCEPT ) {
        filename = gtk_file_chooser_get_filename ( GTK_FILE_CHOOSER ( file_chooser ) );
        load_file ();
    }

    gtk_widget_destroy ( file_chooser );
}

static int
can_open_file ( void ) {
    int prompt;

    if ( gtk_text_buffer_get_modified ( buffer ) == FALSE ) {
        return 1;
    }

    prompt = unsaved_dialog ();
    if ( prompt == YES ) {
        prompt = activate_button_save ();
    }
    return ( prompt != CANCEL );
}

static void
activate_button_open ( void ) {
    if ( can_open_file () ) {
        run_file_chooser_load_file ();
    }
}

static void
create_button_open ( GtkWidget *menu_bar ) {
    GtkWidget *open;

    open = gtk_menu_item_new_with_label ( "Open" );
    g_signal_connect (
        G_OBJECT ( open ),
        "button-press-event",
        G_CALLBACK ( activate_button_open ),
        NULL
    );
    gtk_menu_shell_append ( GTK_MENU_SHELL ( menu_bar ), open );
}

static void
create_button_save ( GtkWidget *menu_bar ) {
    GtkWidget *save;

    save = gtk_menu_item_new_with_label ( "Save" );
    g_signal_connect (
        G_OBJECT ( save ),
        "button-press-event",
        G_CALLBACK ( activate_button_save ),
        NULL
    );
    gtk_menu_shell_append ( GTK_MENU_SHELL ( menu_bar ), save );
}

static void
create_button_saveas ( GtkWidget *menu_bar ) {
    GtkWidget *saveas;

    saveas = gtk_menu_item_new_with_label ( "SaveAs" );
    g_signal_connect (
        G_OBJECT ( saveas ),
        "button-press-event",
        G_CALLBACK ( activate_button_saveas ),
        NULL
    );
    gtk_menu_shell_append ( GTK_MENU_SHELL ( menu_bar ), saveas );
}

static GtkWidget *
create_menu_bar ( void ) {
    GtkWidget *menu_bar;

    menu_bar = gtk_menu_bar_new ();

    create_button_open ( menu_bar );
    create_button_save ( menu_bar );
    create_button_saveas ( menu_bar );

    return menu_bar;
}

static void
generate_css_string ( char *str, size_t size ) {
    const char *font_family;
    gint        font_size;

    font_family = pango_font_description_get_family ( font_description );
    font_size   = pango_font_description_get_size ( font_description );

    (void)snprintf (
        str,
        size,
        "textview {"
        "  font-family: %s;"
        "  font-size:   %dpt;"
        "}",
        font_family,
        ( font_size / PANGO_SCALE )
    );
}

static void
text_view_set_font ( GtkWidget *text_view ) {
    GtkStyleContext *style_context;
    char             css_string[CSS_STRING_MAXLEN];

    css_provider = gtk_css_provider_new ();

    generate_css_string ( css_string, CSS_STRING_MAXLEN );
    gtk_css_provider_load_from_data ( css_provider, css_string, -1, 0 );

    style_context = gtk_widget_get_style_context ( text_view );

    gtk_style_context_add_provider (
        style_context,
        GTK_STYLE_PROVIDER ( css_provider ),
        GTK_STYLE_PROVIDER_PRIORITY_APPLICATION
    );
}

static GtkWidget *
create_text_view ( void ) {
    GtkWidget *scrolled_window = gtk_scrolled_window_new ( NULL, NULL );

    text_view = gtk_text_view_new ();
    gtk_text_view_set_wrap_mode ( GTK_TEXT_VIEW ( text_view ), GTK_WRAP_WORD );
    text_view_set_font ( text_view );
    gtk_widget_set_hexpand ( text_view, TRUE );
    gtk_widget_set_vexpand ( text_view, TRUE );

    gtk_container_add ( GTK_CONTAINER ( scrolled_window ), text_view );
    buffer = gtk_text_view_get_buffer ( GTK_TEXT_VIEW ( text_view ) );
    return scrolled_window;
}

static void
activate ( GtkApplication *app ) {
    GtkWidget *grid;
    GtkWidget *menu_bar;

    builder          = gtk_builder_new ();
    font_description = pango_font_description_from_string ( default_font );

    window = gtk_application_window_new ( app );
    gtk_window_set_title ( GTK_WINDOW ( window ), "notep" );
    gtk_window_set_default_size ( GTK_WINDOW ( window ), SVGA_WIDTH, SVGA_HEIGHT );
    g_signal_connect ( window, "delete-event", G_CALLBACK ( exit_notep ), NULL );

    grid = gtk_grid_new ();
    gtk_container_add ( GTK_CONTAINER ( window ), grid );

    menu_bar = create_menu_bar ();
    gtk_grid_attach ( GTK_GRID ( grid ), menu_bar, 0, 0, 1, 1 );

    text_window = create_text_view ();
    gtk_grid_attach_next_to (
        GTK_GRID ( grid ), text_window, menu_bar, GTK_POS_BOTTOM, 1, 1
    );

    gtk_widget_show_all ( window );
}

int
main ( int argc, char *argv[] ) {
    GtkApplication *app;
    int             status;

    app = gtk_application_new ( "byllgrim.notep", G_APPLICATION_DEFAULT_FLAGS );
    g_signal_connect ( app, "activate", G_CALLBACK ( activate ), NULL );
    status = g_application_run ( G_APPLICATION ( app ), argc, argv );
    g_object_unref ( app );

    return status;
}
