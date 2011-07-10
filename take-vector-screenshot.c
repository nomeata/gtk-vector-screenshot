/*
 * © 2011 Joachim Breitner <mail@joachim-breitner.de>
 *
 *  This library is free software; you can redistribute it and/or
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


#include <gtk/gtk.h>
#include <gdk/gdkx.h>

#include <X11/Xlib.h>
#include <X11/Xutil.h>

#include <clientwin.h>

GdkAtom pdfscreenshot_atom;
char *supported_str = "supported";

GtkWindow *main_window;

/*
 * This is called once a window has been clicked.
 */
gboolean
pdfscreenshot_window_selected(GtkWidget *grab_window,
                  GdkEventButton *event,
                  GtkButton *button)
{
    // Release the pointer
    gdk_pointer_ungrab(event->time);
    // Un-Press the button (we inhibited the button-release signal in
    // pdfscreenshot_select_window
    gtk_button_released(GTK_BUTTON(button));

    // Use Xlib to find out what window is below the button
    Window selected_window = None;
    int dummy;
    unsigned int dummyU;
    Window dummyW;
    XQueryPointer(gdk_x11_get_default_xdisplay(),
        gdk_x11_get_default_root_xwindow(),
        &dummyW, &selected_window, &dummy, &dummy, &dummy, &dummy, &dummyU);

	// Find the "right" window the same way xwininfo does
    selected_window = Find_Client(gdk_x11_get_default_xdisplay(),
        gdk_x11_get_default_root_xwindow(),
        selected_window);

    if (selected_window != None) {
        // Now we check if the window has the GTK_VECTOR_SCREENSHOT atom set
        XTextProperty supported;
        XGetTextProperty(gdk_x11_get_default_xdisplay(),
            selected_window, 
            &supported,
            gdk_x11_atom_to_xatom(pdfscreenshot_atom));
        if (supported.value != NULL) {
            // Send the GTK_VECTOR_SCREENSHOT ClientMessage to the window. It
            // is important to set xevent.window, as X does not do it by
            // itself.
            XClientMessageEvent xevent;
            xevent.type = ClientMessage;
            xevent.message_type = gdk_x11_atom_to_xatom(pdfscreenshot_atom);
            xevent.format = 32;
            xevent.window  = selected_window;
            XSendEvent(gdk_x11_get_default_xdisplay(), selected_window, 0, 0, (XEvent *)&xevent);
            return TRUE;
        }
    }

    // If we reach here, then the window does not support taking vector screenshots.
    GtkWidget *dialog = gtk_message_dialog_new (main_window,
         GTK_DIALOG_DESTROY_WITH_PARENT,
         GTK_MESSAGE_ERROR,
         GTK_BUTTONS_CLOSE,
         "The selected window does not support taking vector screenshots. Is it "
         "a gtk-3 based application, and did you load the gtk-vector-screenshot "
         "module?");
    gtk_dialog_run (GTK_DIALOG (dialog));
    gtk_widget_destroy (dialog);

    return TRUE;
}


/*
 * Called when the main button is pressed.
 */
gboolean
pdfscreenshot_select_window(GtkWidget *button, GdkEvent *event, GtkWidget *grab_window)
{
    GdkEventMask events = GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK;

    // Create a crosshair cursor
    GdkCursor *cursor = gdk_cursor_new_for_display(
        gtk_widget_get_display(button), GDK_CROSSHAIR);
    // And grab the input device, so that all further events are passed to the
    // grab_window
    gdk_device_grab(
        gdk_event_get_device(event),
        gtk_widget_get_window(grab_window),
        GDK_OWNERSHIP_APPLICATION,
        FALSE,
        events,
        cursor,
        event->button.time);

    gdk_cursor_unref(cursor);

    return TRUE;
}


/*
 * The main window, with the one button.
 */
void
pdfscreenshot_window_create()
{
    // Is that icon always installed or do we have to ship it?
    GtkWidget *icon = gtk_image_new_from_icon_name ("camera",GTK_ICON_SIZE_BUTTON);
    gtk_image_set_pixel_size(GTK_IMAGE(icon), 128);

    GtkWidget *button = gtk_button_new_with_label("Take vector screenshot...");
    gtk_button_set_image (GTK_BUTTON(button), icon);
    gtk_button_set_image_position (GTK_BUTTON(button), GTK_POS_TOP);
    gtk_widget_set_tooltip_text (button,
        "Click this button and then an application window to take "
        "a vector screenshot of it.");

    GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_icon_name(GTK_WINDOW(window),"camera");
    gtk_window_set_title(GTK_WINDOW(window),"Vector screenshot taker");
    gtk_window_set_resizable(GTK_WINDOW(window), FALSE);
    gtk_window_set_keep_above(GTK_WINDOW(window),TRUE);
    gtk_container_add(GTK_CONTAINER(window), button);
    gtk_widget_show_all(GTK_WIDGET(window));
    main_window = window;

    // Create a dummy window for the pointer grab
    // (maybe try to remove this later)
    GdkEventMask events = GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK | GDK_POINTER_MOTION_MASK;

    GtkWidget *grab_window = gtk_window_new(GTK_WINDOW_POPUP);
    gtk_widget_show(grab_window);
    gtk_window_resize(GTK_WINDOW(grab_window), 1, 1);
    gtk_window_move(GTK_WINDOW(grab_window), -100, -100);
    gtk_widget_add_events(grab_window, events);

    g_signal_connect(G_OBJECT(grab_window), "button_release_event",
        G_CALLBACK(pdfscreenshot_window_selected), button);
    g_signal_connect(G_OBJECT(button), "button_release_event",
        G_CALLBACK(pdfscreenshot_select_window), grab_window);

}

int
main(gint argc, char *argv[])
{
    gtk_init(&argc, &argv);

    pdfscreenshot_atom = gdk_atom_intern ("GTK_VECTOR_SCREENSHOT", FALSE);
    pdfscreenshot_window_create();

    gtk_main();

    return 0;
}

