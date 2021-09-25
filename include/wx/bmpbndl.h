///////////////////////////////////////////////////////////////////////////////
// Name:        wx/bmpbndl.h
// Purpose:     Declaration of wxBitmapBundle class.
// Author:      Vadim Zeitlin
// Created:     2021-09-22
// Copyright:   (c) 2021 Vadim Zeitlin <vadim@wxwidgets.org>
// Licence:     wxWindows licence
///////////////////////////////////////////////////////////////////////////////

#ifndef _WX_BMPBNDL_H_
#define _WX_BMPBNDL_H_

#include "wx/bitmap.h"
#include "wx/object.h"
#include "wx/vector.h"

class wxBitmapBundleImpl;

// ----------------------------------------------------------------------------
// wxBitmapBundle provides 1 or more versions of a bitmap, all bundled together
// ----------------------------------------------------------------------------

// This class has value semantics and can be copied cheaply.

class WXDLLIMPEXP_CORE wxBitmapBundle
{
public:
    // Default ctor constructs an empty bundle which can't be used for
    // anything, but can be assigned something later.
    wxBitmapBundle();

    // This conversion ctor from a single bitmap does the same thing as
    // FromBitmap() and only exists for interoperability with the existing code
    // using wxBitmap.
    wxBitmapBundle(const wxBitmap& bitmap);

    // Another conversion ctor from a single image: this one is needed to allow
    // passing wxImage to the functions that used to take wxBitmap but now take
    // wxBitmapBundle.
    wxBitmapBundle(const wxImage& image);

    // Default copy ctor and assignment operator and dtor would be ok, but need
    // to be defined out of line, where wxBitmapBundleImpl is fully declared.

    wxBitmapBundle(const wxBitmapBundle& other);
    wxBitmapBundle& operator=(const wxBitmapBundle& other);

    ~wxBitmapBundle();


    // Create from the given collection of bitmaps (all of which must be valid,
    // but if the vector itself is empty, empty bundle is returned).
    static wxBitmapBundle FromBitmaps(const wxVector<wxBitmap>& bitmaps);
    static wxBitmapBundle FromBitmaps(const wxBitmap& bitmap1,
                                      const wxBitmap& bitmap2);

    // Create from a single bitmap (this is only useful for compatibility
    // with the existing code). Returns empty bundle if bitmap is invalid.
    static wxBitmapBundle FromBitmap(const wxBitmap& bitmap);
    static wxBitmapBundle FromImage(const wxImage& image);


    // Check if bitmap bundle is non-empty.
    bool IsOk() const { return m_impl; }

    // Get the size of the bitmap represented by this bundle when using the
    // default DPI, i.e. 100% scaling. Returns invalid size for empty bundle.
    wxSize GetDefaultSize() const;

    // Get bitmap of the specified size, creating a new bitmap from the closest
    // available size by rescaling it if necessary.
    //
    // If size == wxDefaultSize, GetDefaultSize() is used for it instead.
    wxBitmap GetBitmap(const wxSize size) const;

private:
    typedef wxObjectDataPtr<wxBitmapBundleImpl> wxBitmapBundleImplPtr;

    // Private ctor used by static factory functions to create objects of this
    // class. It takes ownership of the pointer (which must be non-null).
    explicit wxBitmapBundle(wxBitmapBundleImpl* impl);

    wxBitmapBundleImplPtr m_impl;
};

// Inline functions implementation.

/* static */ inline
wxBitmapBundle wxBitmapBundle::FromBitmaps(const wxBitmap& bitmap1,
                                           const wxBitmap& bitmap2)
{
    wxVector<wxBitmap> bitmaps;
    if ( bitmap1.IsOk() )
        bitmaps.push_back(bitmap1);
    if ( bitmap2.IsOk() )
        bitmaps.push_back(bitmap2);
    return FromBitmaps(bitmaps);
}

/* static */ inline
wxBitmapBundle wxBitmapBundle::FromBitmap(const wxBitmap& bitmap)
{
    if ( !bitmap.IsOk() )
        return wxBitmapBundle();

    wxVector<wxBitmap> bitmaps;
    bitmaps.push_back(bitmap);
    return FromBitmaps(bitmaps);
}

/* static */ inline
wxBitmapBundle wxBitmapBundle::FromImage(const wxImage& image)
{
    if ( !image.IsOk() )
        return wxBitmapBundle();

    return FromBitmap(wxBitmap(image));
}

#endif // _WX_BMPBNDL_H_
