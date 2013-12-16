// -----------------------------------------------------------------------------
// 2009-2013 by Jupiter Jazz Limited.
//
// This software, excluded third party dependencies, is released in public domain,
// see unlicense.txt file for more detail.
//
// IMPORTATNT:
// NUKE is a trademark of The Foundry Visionmongers Ltd.
// Qt is a trademark of Digia Plc and/or its subsidiary(-ies).
// -----------------------------------------------------------------------------

#ifndef HIERARCHY_VIEW_KNOB_H
#define HIERARCHY_VIEW_KNOB_H

/// dll declaration for windows
#ifdef ATOM_DLL_SPEC
    #undef ATOM_DLL_SPEC
#endif

#ifdef _WIN32

    #ifdef ATOM_DLL_EXPORT
        #define ATOM_DLL_SPEC __declspec(dllexport)
    #elif defined ATOM_DLL_IMPORT
        #define ATOM_DLL_SPEC __declspec(dllimport)
    #else
        #define ATOM_DLL_SPEC
    #endif

#else

    #define ATOM_DLL_SPEC

#endif

#include <DDImage/ddImageVersionNumbers.h>
#include <DDImage/Knob.h>

class HierarchyViewKnobImp;

class ATOM_DLL_SPEC HierarchyViewKnob : public DD::Image::Knob
{
public:
    HierarchyViewKnob( DD::Image::Knob_Closure* _kc, const char** _data, const char* _name, const char* _label = 0 );
    virtual ~HierarchyViewKnob();
    virtual const char* Class() const;
    virtual bool not_default () const;
    virtual void to_script (std::ostream& _os, const DD::Image::OutputContext* _oc, bool _quote) const;
    virtual bool from_script(const char * v);
    virtual void store( DD::Image::StoreType _type, void* _data, DD::Image::Hash& _hash, const DD::Image::OutputContext& _oc);
    virtual const char* get_text( const DD::Image::OutputContext* _oc = 0 ) const;

#if kDDImageVersionInteger < 70000
    virtual WidgetPointer make_widget();
#else
    virtual WidgetPointer make_widget(const DD::Image::WidgetContext& _context);
#endif

public:
    /// wrapper function for addCallback() and removeCallback()
    void addCB( DD::Image::Knob::Callback _cb, void* _closure );
    void removeCB( DD::Image::Knob::Callback _cb, void* _closure );
public:
    /// extended member functions

    /// set header text
    void setHeader( const char* _text );
    /// get and set state of the index of flattened hierarchy, this index is the
    /// same as the visually index in the knob
    void setState( int _idx, int _v );
    int  getState( int _idx ) const;
    /// get and set state of the index of original items ( before expaneding to
    /// be a hierarchy )
    void setItemState( int _idx, int _v );
    int  getItemState( int _idx ) const;
    /// clear the widget, NOTE: the state string in knob does not clear
    /// automatically, clear the string by calling knob("...")->set_text() if
    /// you want to keep data synchronized
    void clear();
    /// reset the items
    /// '_items' is a 2 dim array, you could construct this manually, or use
    /// createItemList(), destroyItemList(), appendItem(), getItemSize() and
    /// getItemList() to manipulate the array; 'ItemList' is provided for
    /// convinence when construct and destruct the array.
    /// '_sep' is a path separator, e.g. '/' for Alembic & Houdini, '|' for Maya,
    /// '_sep' == '\0' indicates that the widget won't build the hierarchy
    /// automatically, but leave the original inputs as they are.
    /// '_states' is the predefined state string, the result states will try to
    /// match this argument if possible, passing a NULL or a size less then the
    /// result items is possible, then '_defaultState' is used when create the
    /// item
    void reset( const char* const* _items, int _itemLen, char _sep, const char* _states, int _defaultState );
public:
    /// helper function to create an item list, the implementation behind is a
    /// std::vector< const char* >, but to simplify the interface and runtime
    /// dependency, a 'void*' is used.
    /// The returned 2 dim array from getItemList() can be passed into reset()
    /// function.
    static void* createItemList();
    static void  destroyItemList( void* _itemList );
    static void  appendItem( void* _itemList, const char* _item );
    static void  reserve( void* _itemList, int _len );
    static int   getItemSize( void* _itemList );
    static const char* const* getItemList( void* _itemList );
public:
    /// helper class, make the array works in a 'smart pointer' manner
    struct ItemList {
        void* data;
        ItemList() : data( HierarchyViewKnob::createItemList() ) {}
        ~ItemList() { HierarchyViewKnob::destroyItemList( data ); }
        inline void appendItem( const char* _item ) { HierarchyViewKnob::appendItem( this->data, _item ); }
        inline void reserve( int _len ) { HierarchyViewKnob::reserve( this->data, _len ); }
        inline int getItemSize() { return HierarchyViewKnob::getItemSize( this->data ); }
        inline const char* const* getItemList() { return HierarchyViewKnob::getItemList( this->data ); }
    };
private:
    /// detail implementation, the class definition can be found in
    /// HierarchyViewKnob.cpp file
    HierarchyViewKnobImp* impl_;
};

#endif
