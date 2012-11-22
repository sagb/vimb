#include "dom.h"
#include "main.h"

static gboolean dom_auto_insert(WebKitDOMElement* element);
static gboolean dom_editable_focus_cb(WebKitDOMElement* element, WebKitDOMEvent* event);
static gboolean dom_is_editable(WebKitDOMElement* element);
static WebKitDOMElement* dom_get_active_element(WebKitDOMDocument* doc);


void dom_check_auto_insert(void)
{
    WebKitDOMDocument* doc   = webkit_web_view_get_dom_document(vp.gui.webview);
    WebKitDOMElement* active = dom_get_active_element(doc);
    if (!dom_auto_insert(active)) {
        WebKitDOMHTMLElement *element = webkit_dom_document_get_body(doc);
        if (!element) {
            element = WEBKIT_DOM_HTML_ELEMENT(webkit_dom_document_get_document_element(doc));
        }
        webkit_dom_event_target_add_event_listener(
            WEBKIT_DOM_EVENT_TARGET(element), "focus", G_CALLBACK(dom_editable_focus_cb), true, NULL
        );
    }
}

static gboolean dom_auto_insert(WebKitDOMElement* element)
{
    if (dom_is_editable(element)) {
        Arg a = {VP_MODE_INSERT};
        vp_set_mode(&a);
        return TRUE;
    }
    return FALSE;
}

static gboolean dom_editable_focus_cb(WebKitDOMElement* element, WebKitDOMEvent* event)
{
    webkit_dom_event_target_remove_event_listener(
        WEBKIT_DOM_EVENT_TARGET(element), "focus", G_CALLBACK(dom_editable_focus_cb), true
    );
    if (CLEAN_MODE(vp.state.mode) != VP_MODE_INSERT) {
        WebKitDOMEventTarget* target = webkit_dom_event_get_target(event);
        dom_auto_insert((void*)target);
    }
    return FALSE;
}

/**
 * Indicates if the given dom element is an editable element like text input,
 * password or textarea.
 */
static gboolean dom_is_editable(WebKitDOMElement* element)
{
    if (!element) {
        return FALSE;
    }

    gchar* tagname = webkit_dom_node_get_node_name(WEBKIT_DOM_NODE(element));
    if (!g_strcmp0(tagname, "TEXTAREA")) {
        return TRUE;
    }
    if (!g_strcmp0(tagname, "INPUT")) {
        gchar *type = webkit_dom_element_get_attribute((void*)element, "type");
        if (!g_strcmp0(type, "text") || !g_strcmp0(type, "password")) {
            return TRUE;
        }
    }
    return FALSE;
}

static WebKitDOMElement* dom_get_active_element(WebKitDOMDocument* doc)
{
    WebKitDOMDocument* d     = NULL;
    WebKitDOMElement* active = webkit_dom_html_document_get_active_element((void*)doc);
    gchar* tagname           = webkit_dom_element_get_tag_name(active);

    if (!g_strcmp0(tagname, "FRAME")) {
        d   = webkit_dom_html_frame_element_get_content_document(WEBKIT_DOM_HTML_FRAME_ELEMENT(active));
        return dom_get_active_element(d);
    }
    if (!g_strcmp0(tagname, "IFRAME")) {
        d   = webkit_dom_html_iframe_element_get_content_document(WEBKIT_DOM_HTML_IFRAME_ELEMENT(active));
        return dom_get_active_element(d);
    }
    return active;
}
