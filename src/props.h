#pragma once

#include <wingdi.h>
#include "napi.h"

class DocProps
{
public:
  enum DocumentOrientation
  {
    PORTRAIT,
    LANDSCAPE
  };

  DocProps() {}

  DocProps(PDEVMODEW devMode) : m_devMode(devMode) {}

  DocumentOrientation getOrientation() const
  {
    return m_orientation == DMORIENT_PORTRAIT ? DocumentOrientation::PORTRAIT : DocumentOrientation::LANDSCAPE;
  }

  double getPaperWidth() const
  {
    return (double)m_devMode->dmPaperWidth / 10;
  }

  double getPaperHeight() const
  {
    return (double)m_devMode->dmPaperLength / 10;
  }

  double getDPI() const
  {
    return (double)m_devMode->dmPrintQuality;
  }

  short getCopies() const
  {
    return m_devMode->dmCopies;
  }

  PDEVMODEW getDevMode() const
  {
    return m_devMode;
  }

  void setCopies(short copies)
  {
    if (m_devMode == nullptr)
      return;

    short m_copies = copies > 0 ? copies : 0;
    m_devMode->dmCopies = m_copies;
  }

  void setOrientation(DocumentOrientation orientation)
  {
    if (m_devMode == nullptr)
      return;

    m_devMode->dmOrientation = m_orientation == DocumentOrientation::PORTRAIT ? DMORIENT_PORTRAIT : DMORIENT_LANDSCAPE;
  }

  void setPaperWidth(double paperWidth)
  {
    if (m_devMode == nullptr)
      return;

    double m_paperWidth = paperWidth > 0 ? paperWidth : 0;
    m_devMode->dmPaperWidth = (short)(m_paperWidth * 10);
  }

  void setPaperHeight(double paperHeight)
  {
    if (m_devMode == nullptr)
      return;

    double m_paperHeight = paperHeight > 0 ? paperHeight : 0;
    m_devMode->dmPaperLength = (short)(m_paperHeight * 10);
  }

  void setScale(double scale)
  {
    if (m_devMode == nullptr)
      return;

    double m_scale = scale > 0 ? scale : 0;
    m_devMode->dmScale = (short)(m_scale * 100);
  }

  void setDPI(double dpi)
  {
    if (m_devMode == nullptr)
      return;

    double m_dpi = dpi > 0 ? dpi : 0;
    m_devMode->dmPrintQuality = (short)m_dpi;
  }

  void fromObject(const Napi::Object &object)
  {
    if (object.Has("copies"))
    {
      setCopies(object.Get("copies").ToNumber().Int32Value());
    }

    if (object.Has("orientation"))
    {
      setOrientation(
          object.Get("orientation").ToString().Utf8Value() == "portrait" ? DocumentOrientation::PORTRAIT : DocumentOrientation::LANDSCAPE);
    }

    if (object.Has("paperWidth"))
    {
      setPaperWidth(object.Get("paperWidth").ToNumber().DoubleValue());
    }

    if (object.Has("paperHeight"))
    {
      setPaperHeight(object.Get("paperHeight").ToNumber().DoubleValue());
    }

    if (object.Has("scale"))
    {
      setScale(object.Get("scale").ToNumber().DoubleValue());
    }

    if (object.Has("dpi"))
    {
      setDPI(object.Get("dpi").ToNumber().DoubleValue());
    }
  }

  Napi::Object toObject(Napi::Env env)
  {
    Napi::Object ret = Napi::Object::New(env);

    ret.Set("copies", getCopies());
    ret.Set("orientation",
            getOrientation() ? Napi::String::From(env, "portrait") : Napi::String::From(env, "landscape"));
    ret.Set("paperWidth", getPaperWidth());
    ret.Set("paperHeight", getPaperHeight());
    ret.Set("dpi", getDPI());

    return ret;
  }

private:
  PDEVMODEW m_devMode = nullptr;
  DocumentOrientation m_orientation = DocumentOrientation::PORTRAIT;
};