/*
 ============================================================================
 Name        : gst-amc-sink.c
 Author      : Heiher <r@hev.cc>
 Version     : 0.0.1
 Copyright   : Copyright (C) 2014 everyone.
 Description : 
 ============================================================================
 */

#include "gst-amc-sink.h"

GST_DEBUG_CATEGORY_STATIC (gst_amc_sink_debug);
#define GST_CAT_DEFAULT gst_amc_sink_debug

static gboolean gst_amc_sink_start (GstBaseSink *base_sink);
static gboolean gst_amc_sink_stop (GstBaseSink *base_sink);
static void gst_amc_sink_get_times (GstBaseSink * base_sink, GstBuffer * buf,
            GstClockTime * start, GstClockTime * end);
static GstFlowReturn gst_amc_sink_render (GstBaseSink *base_sink,
            GstBuffer *buffer);

static GstStaticPadTemplate gst_amc_sink_sink_template =
GST_STATIC_PAD_TEMPLATE (
            "sink",
            GST_PAD_SINK,
            GST_PAD_ALWAYS,
            GST_STATIC_CAPS ("video/x-amc-direct"));

#define PTS_DELTA (40 * GST_MSECOND)
#define gst_amc_sink_parent_class parent_class
G_DEFINE_TYPE (GstAmcSink, gst_amc_sink, GST_TYPE_BASE_SINK);

static void
gst_amc_sink_dispose (GObject *obj)
{
    G_OBJECT_CLASS (parent_class)->dispose (obj);
}

static void
gst_amc_sink_finalize (GObject *obj)
{
    G_OBJECT_CLASS (parent_class)->finalize (obj);
}

static GObject *
gst_amc_sink_constructor (GType type, guint n, GObjectConstructParam *param)
{
    return G_OBJECT_CLASS (parent_class)->constructor (type, n, param);
}

static void
gst_amc_sink_constructed (GObject *obj)
{
    G_OBJECT_CLASS (parent_class)->constructed (obj);
}

static void
gst_amc_sink_class_init (GstAmcSinkClass *klass)
{
    GObjectClass *obj_class = G_OBJECT_CLASS (klass);
    GstElementClass *element_class = GST_ELEMENT_CLASS (klass);
    GstBaseSinkClass *base_sink_class = GST_BASE_SINK_CLASS (klass);

    obj_class->constructor = gst_amc_sink_constructor;
    obj_class->constructed = gst_amc_sink_constructed;
    obj_class->dispose = gst_amc_sink_dispose;
    obj_class->finalize = gst_amc_sink_finalize;

    base_sink_class->start = GST_DEBUG_FUNCPTR (gst_amc_sink_start);
    base_sink_class->stop = GST_DEBUG_FUNCPTR (gst_amc_sink_stop);
    base_sink_class->get_times = GST_DEBUG_FUNCPTR (gst_amc_sink_get_times);
    base_sink_class->render = GST_DEBUG_FUNCPTR (gst_amc_sink_render);
    base_sink_class->preroll = GST_DEBUG_FUNCPTR (gst_amc_sink_render);

    gst_element_class_add_pad_template (element_class,
            gst_static_pad_template_get (&gst_amc_sink_sink_template));
    gst_element_class_set_static_metadata (element_class, "Amc Sink",
            "Sink/Amc",
            "Android Media codec sink",
            "Heiher <r@hev.cc>");

    GST_DEBUG_CATEGORY_INIT (GST_CAT_DEFAULT, "amcsink", 0, "AmcSink");
}

static void
gst_amc_sink_init (GstAmcSink *self)
{
    gst_base_sink_set_max_lateness (GST_BASE_SINK (self), 80 * GST_MSECOND);
    gst_base_sink_set_qos_enabled (GST_BASE_SINK (self), TRUE);
}

static gboolean
gst_amc_sink_start (GstBaseSink *base_sink)
{
    return TRUE;
}

static gboolean
gst_amc_sink_stop (GstBaseSink *base_sink)
{
    return TRUE;
}

static void
gst_amc_sink_get_times (GstBaseSink * base_sink, GstBuffer * buf,
            GstClockTime * start, GstClockTime * end)
{
    if (GST_BUFFER_TIMESTAMP_IS_VALID (buf)) {
        *start = GST_BUFFER_TIMESTAMP (buf) - PTS_DELTA;
        if (GST_BUFFER_DURATION_IS_VALID (buf))
            *end = *start + GST_BUFFER_DURATION (buf);
        else
            *end = GST_CLOCK_TIME_NONE;
    }
}

static GstFlowReturn
gst_amc_sink_render (GstBaseSink *base_sink,
            GstBuffer *buffer)
{
    GstAmcSink *self = GST_AMC_SINK (base_sink);
    GstAmcSinkBufferData *buffer_data;
    GError *error = NULL;
    GstMapInfo map_info;

    if (!gst_buffer_map (buffer, &map_info, GST_MAP_READ))
      return GST_FLOW_ERROR;

    buffer_data = (GstAmcSinkBufferData *) map_info.data;
    if (buffer_data->codec) {
        if (gst_amc_codec_release_output_buffer (buffer_data->codec,
                        buffer_data->index, TRUE, PTS_DELTA, &error)) {
            buffer_data->codec = NULL;
        } else {
            GST_ERROR_OBJECT (self, "Release output buffer fail: %s",
                        error->message);
            g_error_free (error);
        }
    }

    gst_buffer_unmap (buffer, &map_info);

    return GST_FLOW_OK;
}

/* vim:set tabstop=4 softtabstop=4 shiftwidth=4 expandtab: */
