/*
 * © 2011 Joachim Breitner <mail@joachim-breitner.de>
 *
 *   This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */


// For strchrnul
#define _GNU_SOURCE

#include <math.h>
#include <string.h>
#include <libgen.h>

#include <gtk/gtk.h>

#include <cairo-pdf.h>
#include <cairo-svg.h>
#include <cairo-ps.h>

void pdfscreenshot_type_selected(GtkComboBox *format_combo, GtkFileChooser *chooser) {
    const char *the_id = gtk_combo_box_get_active_id(GTK_COMBO_BOX(format_combo));

    // Update filter
    GtkFileFilter *filter = gtk_file_filter_new ();
    char *pattern = g_strdup_printf("*.%s", the_id);
    gtk_file_filter_add_pattern (filter, pattern);
    gtk_file_chooser_set_filter (GTK_FILE_CHOOSER(chooser), filter);

    // Update filename extension
    gchar *filename = gtk_file_chooser_get_filename(chooser);
    if (filename != NULL)  {
        // strup result from basename as we are going to modify it.
        char *base = g_strdup(basename(filename));
        // Remove extension from string, if there is one.
        *(strchrnul(base, '.')) = '\0';
        
        char *new_filename = g_strdup_printf("%s.%s", base, the_id);
        gtk_file_chooser_set_current_name(chooser, new_filename);
        g_free(filename);
        g_free(base);
        g_free(new_filename);
    }
}

void pdfscreenshot_draw_preview (GtkWidget *widget, cairo_t *cr, gpointer window) {

    int draw_width = gtk_widget_get_allocated_width (widget);
    int draw_height = gtk_widget_get_allocated_height (widget);
    int win_width = gtk_widget_get_allocated_width (window);
    int win_height = gtk_widget_get_allocated_height (window);

    double scale = fmin(1, fmin (1.0*draw_width / win_width, 1.0*draw_height / win_height));

    cairo_scale(cr, scale, scale);

    cairo_translate(cr, (draw_width  - scale * win_width)/2,
                        (draw_height - scale * win_height)/2);

    gtk_widget_draw(window, cr);
}

void pdfscreenshot_draw_to_vector (GtkWidget *widget, const gchar* filename, cairo_surface_t * create_surface(const char *, double , double )) {
    cairo_surface_t *surface = 
            create_surface(filename,
                    1.0*gtk_widget_get_allocated_width (widget),
                    1.0*gtk_widget_get_allocated_height (widget));
    cairo_t *cr  = cairo_create(surface);
    gtk_widget_draw(widget, cr);
    cairo_destroy(cr);
    cairo_surface_destroy(surface);
}

void pdfscreenshot_draw_to_png (GtkWidget *widget, const gchar* filename) {
    cairo_surface_t *surface = cairo_image_surface_create (
                    CAIRO_FORMAT_ARGB32,
                    gtk_widget_get_allocated_width (widget),
                    gtk_widget_get_allocated_height (widget));
    cairo_t *cr  = cairo_create(surface);
    gtk_widget_draw(widget, cr);
    cairo_destroy(cr);
    cairo_surface_write_to_png (surface, filename);
    cairo_surface_destroy(surface);
}

void
pdfscreenshot_take_shot (GtkButton *button, gpointer our_window) {
    GList *toplevels = gtk_window_list_toplevels();

    for (GList *iter = toplevels; iter; iter = g_list_next(iter)) {
        GtkWindow *window = GTK_WINDOW(iter->data);
        const gchar *title = gtk_window_get_title(window);
        if (our_window != window && title != NULL) {
            // This seems to be the "other" window
            g_object_ref(window);

            GtkWidget *chooser = gtk_file_chooser_dialog_new (
                "Save vector screenshot",
                our_window,
                GTK_FILE_CHOOSER_ACTION_SAVE,
                GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
                GTK_STOCK_SAVE, GTK_RESPONSE_ACCEPT,
                NULL);

            char *filename = g_strdup_printf("%s.pdf", g_get_application_name());
            gtk_file_chooser_set_current_name (GTK_FILE_CHOOSER (chooser), filename);
            g_free(filename);


            gtk_window_set_transient_for (GTK_WINDOW(chooser), GTK_WINDOW(our_window));
            gtk_file_chooser_set_do_overwrite_confirmation (GTK_FILE_CHOOSER (chooser), TRUE);

            GtkWidget *format_combo = gtk_combo_box_text_new ();
            gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(format_combo)
                                     , "pdf" ,"Save as PDF (*.pdf)");
            gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(format_combo)
                                     , "svg" ,"Save as SVG (*.svg)");
            gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(format_combo)
                                     , "ps" ,"Save as PostScript (*.ps)");
            gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(format_combo)
                                     , "png" ,"Save as PNG (*.png)");
            gtk_combo_box_set_active(GTK_COMBO_BOX(format_combo),0);
            pdfscreenshot_type_selected(GTK_COMBO_BOX(format_combo), GTK_FILE_CHOOSER(chooser));
	    g_signal_connect(GTK_COMBO_BOX(format_combo),"changed",
	    	G_CALLBACK(pdfscreenshot_type_selected), chooser);


            GtkWidget *drawing_area = gtk_drawing_area_new ();
            g_signal_connect (G_OBJECT (drawing_area), "draw",
                    G_CALLBACK (pdfscreenshot_draw_preview), window);
            gtk_widget_set_size_request (drawing_area, 300, 300);
        
            GtkWidget *frame = gtk_frame_new("Preview");
            gtk_container_add(GTK_CONTAINER(frame), drawing_area);

	    GtkWidget *vbox = gtk_vbox_new(FALSE, 8);
            gtk_box_pack_start(GTK_BOX(vbox), format_combo, FALSE, FALSE, 0);
            gtk_box_pack_start(GTK_BOX(vbox), frame, TRUE, TRUE, 0);
            gtk_widget_show_all(vbox);

            gtk_file_chooser_set_extra_widget(GTK_FILE_CHOOSER(chooser), vbox);

            if (gtk_dialog_run (GTK_DIALOG(chooser)) == GTK_RESPONSE_ACCEPT) {
                char *filename = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (chooser));
		//GtkFileFilter *the_filter = gtk_file_chooser_get_filter(GTK_FILE_CHOOSER(chooser));
                const char *the_id = gtk_combo_box_get_active_id(GTK_COMBO_BOX(format_combo));

		if (!g_strcmp0(the_id,"pdf"))
                    pdfscreenshot_draw_to_vector(GTK_WIDGET(window),filename,
                            cairo_pdf_surface_create);
		else if (!g_strcmp0(the_id,"svg"))
                    pdfscreenshot_draw_to_vector(GTK_WIDGET(window),filename,
                            cairo_svg_surface_create);
		else if (!g_strcmp0(the_id,"ps"))
                    pdfscreenshot_draw_to_vector(GTK_WIDGET(window),filename,
                            cairo_ps_surface_create);
		else if (!g_strcmp0(the_id,"png"))
                    pdfscreenshot_draw_to_png(GTK_WIDGET(window),filename);
		else
                    printf("Unknown id \"%s\"\n", the_id);

                g_free(filename);                        
            }

            gtk_widget_destroy (chooser);
            g_object_unref(window);
            g_list_free(toplevels);
            return;
        }
    }
    g_list_free(toplevels);

    GtkWidget *dialog = gtk_message_dialog_new (our_window,
         GTK_DIALOG_DESTROY_WITH_PARENT,
         GTK_MESSAGE_ERROR,
         GTK_BUTTONS_CLOSE,
         "No main window found.");
    gtk_dialog_run (GTK_DIALOG (dialog));
    gtk_widget_destroy (dialog);
}

void
pdfscreenshot_window_create()
{

    GtkWidget *icon = gtk_image_new_from_icon_name ("camera",GTK_ICON_SIZE_BUTTON);
    gtk_image_set_pixel_size(GTK_IMAGE(icon), 128);

    GtkWidget *button = gtk_button_new_with_label("Take vector screenshot...");
    gtk_button_set_image (GTK_BUTTON(button), icon);
    gtk_button_set_image_position (GTK_BUTTON(button), GTK_POS_TOP);

    GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window),"Vector screenshot taker");
    gtk_window_set_resizable(GTK_WINDOW(window), FALSE);
    gtk_window_set_keep_above(GTK_WINDOW(window),TRUE);
    gtk_container_add(GTK_CONTAINER(window), button);

    g_signal_connect(G_OBJECT(button), "clicked",
        G_CALLBACK(pdfscreenshot_take_shot), window);


    gtk_widget_show_all(GTK_WIDGET(window));
}

int
gtk_module_init(gint argc, char *argv[])
{
    pdfscreenshot_window_create();
    return FALSE;
}


