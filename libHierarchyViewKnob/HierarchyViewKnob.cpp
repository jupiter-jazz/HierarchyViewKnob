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

#include "HierarchyViewKnob.h"
#include "HierarchyViewWidget.moc.h"
#include <DDImage/Knob.h>

#include <string.h>

#include <map>
#include <string>
#include <vector>

#include <QStringBuilder>

////////////////////////////////////////////////////////////////////////////////
/// HierarchyViewWidget
////////////////////////////////////////////////////////////////////////////////

HierarchyViewWidget::HierarchyViewWidget( HierarchyViewKnob* _knob ) : knob_( _knob ), absIdxHash_(), idxArray_(), suspendUpdate_( false )
{
    setColumnCount( 1 );
    setHeaderLabel( "" );
    knob_->addCB( WidgetCallback, this );
    connect( this, SIGNAL( itemChanged( QTreeWidgetItem*, int ) ), this, SLOT( valueChanged( QTreeWidgetItem*, int ) ) );
}

HierarchyViewWidget::~HierarchyViewWidget()
{
    if ( knob_ ) {
        knob_->removeCB( WidgetCallback, this );
    }
}

void HierarchyViewWidget::wheelEvent( QWheelEvent* _event )
{
    QScrollBar* scrollBar = NULL;
    if ( _event->orientation() == Qt::Horizontal ) {
        scrollBar = horizontalScrollBar();

    } else if ( _event->orientation() == Qt::Vertical ) {
        scrollBar = verticalScrollBar();

    }

    if ( scrollBar ) {
        scrollBar->setValue( scrollBar->value() - int( _event->delta() / 10.0f ) );
    }
}

bool HierarchyViewWidget::parentItemState( QTreeWidgetItem* parentItem ) const
{
    while ( parentItem ) {
        if ( ! parentItem->checkState( 0 ) ) {
            return false;
        }
        parentItem = parentItem->parent();
    }
    return true;
}

void HierarchyViewWidget::valueChanged( QTreeWidgetItem* _item, int _column )
{
    if ( knob_ && getAbsIndex( _item ) >= 0 ) {

        /// change item state
        bool state(  _item->checkState( _column ) );
        knob_->setState( getAbsIndex( _item ), state );

        /// NOTE: if suspendUpdate == true, there are items added into the widget
        /// so we stop looking for their children items, otherwise the state can
        /// never be correct
        if ( !getSuspendUpdate() ) {

            /// get the full path of current item
            QString fullPath( _item->text( _column ) );
            for ( QTreeWidgetItem* parentItem( _item->parent() ); parentItem; parentItem = parentItem->parent() ) {
                fullPath = parentItem->text( 0 ) % "/" % fullPath;
            }
            fullPath = "/" % fullPath;

            const QVector< QPair< QString, QTreeWidgetItem* > >& idxArray( itemIndices() );
            for ( int idx( 0 ); idx < idxArray.size(); ++idx ) {
                if ( idxArray[ idx ].first == fullPath ) {
                    knob_->setItemState( idx, parentItemState( idxArray[ idx ].second ) );
                } else  if ( idxArray[ idx ].first.startsWith( fullPath % "/" ) ) {
                    if ( idxArray[ idx ].second && idxArray[ idx ].second->checkState( 0 ) ) {
                        knob_->setItemState( idx, parentItemState( idxArray[ idx ].second->parent() ) );
                    }
                }
            }
        }
    }
}

int HierarchyViewWidget::getAbsIndex( QTreeWidgetItem* _item ) const
{
    if ( absIdxHash_.contains( _item ) ) {
        return absIdxHash_.value( _item );
    }
    return -1;
}

void HierarchyViewWidget::setAbsIndex( QTreeWidgetItem* _item, int _idx )
{
    absIdxHash_.insert( _item, _idx );
}

const QVector< QPair< QString, QTreeWidgetItem* > >& HierarchyViewWidget::itemIndices() const
{
    return idxArray_;
}

QVector< QPair< QString, QTreeWidgetItem* > >& HierarchyViewWidget::itemIndices()
{
    return idxArray_;
}

bool HierarchyViewWidget::getSuspendUpdate() const
{
    return suspendUpdate_;
}

void HierarchyViewWidget::setSuspendUpdate( bool _suspendUpdate )
{
    suspendUpdate_ = _suspendUpdate;
}

void HierarchyViewWidget::update()
{
}

void HierarchyViewWidget::destroy()
{
    knob_ = NULL;
}

int HierarchyViewWidget::WidgetCallback( void* _closure, DD::Image::Knob::CallbackReason _reason )
{
    /// could double check if on main thread here just in case and bail out
    /// if not
    HierarchyViewWidget* widget = ( HierarchyViewWidget* )_closure;
    switch ( _reason ) {
        case DD::Image::Knob::kIsVisible:
        {
            /// We check for visibility up to the containing tab widget.
            /// This means that a widget is still considered visible when its
            /// NodePanel is hidden due to being in hidden tab in a dock.
            for ( QWidget* w = widget->parentWidget(); w; w = w->parentWidget() ) {
                if ( qobject_cast< QTabWidget* >( w ) ) {
                    return widget->isVisibleTo( w );
                }
            }
            return widget->isVisible();
        }
        case DD::Image::Knob::kUpdateWidgets:
        {
            widget->update();
            break;
        }
        case DD::Image::Knob::kDestroying:
        {
            widget->destroy();
            break;
        }
        default:
            break;
    }
    return 0;
}

////////////////////////////////////////////////////////////////////////////////
/// HierarchyViewKnobImp
/// This is the actual implementation of HierarchyViewKnob, this class holds
/// all the necessary data of the knob, intents to hide all the detail from
/// the interface ( the header file ), user who wants to use this knob doesn't
/// have to deal with linking Qt and STL.
////////////////////////////////////////////////////////////////////////////////

class HierarchyViewKnobImp
{
public:
    HierarchyViewKnobImp( const char** _data )
        : widget_( NULL ), items_(), allStatesStr_( "" ), itemStatesStr_( "" ), indexMap_()
    {
        if ( _data && (*_data) ) {
            allStatesStr_ = std::string( *_data );
        }
    }

    inline bool not_default () const
    {
        return !items_.empty();
    }

    inline void to_script( std::ostream& _os, const DD::Image::OutputContext* _oc, bool _quote) const
    {
        _os << "[";
        _os << allStatesStr_.c_str();
        _os << ",";
        _os << itemStatesStr_.c_str();
        _os << "]";
    }

    inline bool from_script( const char* _v )
    {
        if ( _v ) {
            allStatesStr_ = "";
            itemStatesStr_ = "";
            std::size_t vLen( ::strlen( _v ) );
            /// NOTE: there is a memory leak and crash here when using QString
            ///       on Windows, a hand-made loop to copy the states seems
            ///       could avoid crashing.
            std::size_t idx( 0 );
            while ( idx < vLen ) {
                if ( _v[ idx ] == ',' ) {
                    ++idx;
                    break;
                }
                if ( _v[ idx ] == '0' || _v[ idx ] == '1' ) {
                    allStatesStr_.push_back( _v[ idx ] );
                }
                ++idx;
            }
            while ( idx < vLen ) {
                if ( _v[ idx ] == ']' ) {
                    ++idx;
                    break;
                }
                if ( _v[ idx ] == '0' || _v[ idx ] == '1' ) {
                    itemStatesStr_.push_back( _v[ idx ] );
                }
                ++idx;
            }
            return true;
        }
        return false;
    }

    inline void store( DD::Image::StoreType _type, void* _data, DD::Image::Hash& _hash, const DD::Image::OutputContext& _oc )
    {
        if ( !allStatesStr_.empty() ) {
            _hash.append( allStatesStr_.c_str() );
        } else {
            _hash.append( "" );
        }
        if ( !itemStatesStr_.empty() ) {
            _hash.append( itemStatesStr_.c_str() );
        } else {
            _hash.append( "" );
        }
        const char** data = ( const char** )( _data );
        *data = allStatesStr_.c_str();
    }

    inline const char* get_text( const DD::Image::OutputContext* _oc ) const
    {
        if ( !allStatesStr_.empty() ) {
            return allStatesStr_.c_str();
        }
        return "";
    }


#if kDDImageVersionInteger < 70000

    inline WidgetPointer make_widget( HierarchyViewKnob* _k )
    {
        widget_ = new HierarchyViewWidget( _k );
        return widget_;
    }

#else

    inline WidgetPointer make_widget( HierarchyViewKnob* _k, const DD::Image::WidgetContext& _context )
    {
        widget_ = new HierarchyViewWidget( _k );
        return widget_;
    }

#endif

public:

    inline void setHeader( const std::string& _text )
    {
        if ( widget_ ) {
            widget_->setHeaderLabel( QString::fromStdString( _text ) );
        }
    }

    inline std::size_t itemSize()
    {
        /// items_.size() should be equal to allStatesStr_.size() !!
        return items_.size();
    }

    inline std::size_t statesSize()
    {
        return allStatesStr_.size();
    }

    inline std::size_t itemStatesSize()
    {
        return itemStatesStr_.size();
    }

    inline bool hasItem( const std::string&  _fullPath ) const
    {
        return _fullPath.size() && indexMap_.find( _fullPath ) != indexMap_.end();
    }

    inline int itemIndex( const std::string& _fullPath ) const
    {
        if ( hasItem( _fullPath ) ) {
#if defined( _MSC_VER ) && ( _MSC_VER <= 1600 )
            return indexMap_.find( _fullPath )->second;
#else
            return indexMap_.at( _fullPath );
#endif
        }
        return -1;
    }

    inline const std::string& itemName( int _idx ) const
    {
        /// caller should handle boundary checking
        return items_[ static_cast< std::size_t >( _idx ) ].first;
    }

    inline const std::string& itemPath( int _idx ) const
    {
        /// caller should handle boundary checking
        return items_[ static_cast< std::size_t >( _idx ) ].second;
    }

    ///-------------------------------------------------------------------

    inline void setState( int _idx, int _v )
    {
        /// caller should handle boundary checking
        allStatesStr_[ static_cast< std::size_t >( _idx ) ] = ( bool( _v ) ? '1' : '0' ) ;
    }

    inline int getState( int _idx ) const
    {
        /// caller should handle boundary checking
        return ( allStatesStr_[ static_cast< std::size_t >( _idx ) ] != '0' ) ;
    }

    inline void setItemState( int _idx, int _v )
    {
        /// caller should handle boundary checking
        itemStatesStr_[ static_cast< std::size_t >( _idx ) ] = ( bool( _v ) ? '1' : '0' ) ;
    }

    inline int getItemState( int _idx ) const
    {
        /// caller should handle boundary checking
        return ( itemStatesStr_[ static_cast< std::size_t >( _idx ) ] != '0' ) ;
    }

    inline void clear()
    {
        items_.clear();
        allStatesStr_.clear();
        itemStatesStr_.clear();
        indexMap_.clear();

        if ( widget_ ) {
            widget_->clear();
        }
    }

    inline void addItemToParent( QTreeWidget* _parent, QTreeWidgetItem* _item )
    {
        _parent->addTopLevelItem( _item );
    }

    inline void addItemToParent( QTreeWidgetItem* _parent, QTreeWidgetItem* _item )
    {
        _parent->addChild( _item );
    }

    template< typename ParentItemT, typename SiblingT >
    inline QTreeWidgetItem* createWidgetItem( ParentItemT _parent, SiblingT _sibling, const QString& _name, const std::string& _fullPath, int _stateLen, const std::string& _states, int _defaultState )
    {
        QTreeWidgetItem* item = NULL;
        /// check if item already been created
        if ( _sibling ) {
            for ( QTreeWidgetItemIterator itemIt( _sibling ); ( *itemIt ); ++itemIt ) {
                if ( ( *itemIt )->text( 0 ) == _name ) {
                    item = ( *itemIt );
                }
            }
        }

        /// create new one if not exists
        if ( !item ) {
            int itemIndex( static_cast< int >( items_.size() ) );

            /// if not found, the root item has not been created yet
            item = new QTreeWidgetItem( ( ParentItemT )0, QStringList( _name ) );
            item->setFlags( item->flags() | Qt::ItemIsUserCheckable );

            widget_->setAbsIndex( item, itemIndex );

            addItemToParent( _parent, item );

            indexMap_[ _fullPath + "/" + _name.toStdString() ] = itemIndex;

            bool state( itemIndex < _stateLen ? int( char( _states[ static_cast< std::size_t >( itemIndex ) ] - '0' ) ) : _defaultState );
            if ( state ) {
                item->setCheckState( 0, Qt::Checked );
            } else {
                item->setCheckState( 0, Qt::Unchecked );
            }

            items_.push_back( std::make_pair( _name.toStdString(), _fullPath + "/" + _name.toStdString() ) );
            allStatesStr_.push_back( state ? '1' : '0' );
        }

        return item;
    }

    inline void reset( const char* const* _items, int _itemLen, char _sep, const char* _states, int _defaultState )
    {
        /// save previous selection state
        std::string states;
        if ( _states ) {
            states = std::string( _states );
        }
        /// clear
        clear();

        /// reset
        if ( widget_ && _items && _itemLen > 0 ) {

            widget_->setSuspendUpdate( true );

            widget_->itemIndices().reserve( _itemLen );

            int stateLen( static_cast< int >( states.size() ) );
            QTreeWidgetItem* parent = NULL;
            std::string fullPath;

            for ( int idx( 0 ); idx < _itemLen; ++idx ) {
                parent = NULL;
                fullPath.clear();

                /// this variable denotes the item state, also considers the
                /// parent nodes
                bool itemState( true );

                QStringList tokens;
                if ( _sep != '\0' ) {
                    tokens = QStringList( QString::fromStdString( _items[ idx ] ).split( _sep, QString::SkipEmptyParts) );
                } else {
                    tokens = QStringList( QString::fromStdString( _items[ idx ] ) );
                }

                for ( int tokenIdx( 0 ); tokenIdx < tokens.size(); ++tokenIdx ) {
                    QString itemStr( tokens.at( tokenIdx ) );

                    if ( !parent ) {
                        parent = createWidgetItem( widget_, widget_, itemStr, fullPath, stateLen, states, _defaultState );
                    } else {
                        parent = createWidgetItem( parent, parent->child( 0 ), itemStr, fullPath, stateLen, states, _defaultState );
                    }

                    if ( parent && itemState && ! parent->checkState( 0 ) ) {
                        itemState = false;
                    }

                    fullPath = fullPath + "/" + itemStr.toStdString();
                }
                widget_->itemIndices().append( QPair< QString, QTreeWidgetItem* >( fullPath.c_str(), parent ) );
                itemStatesStr_.push_back( itemState ? '1' : '0' );
            }
            widget_->expandAll();

            widget_->setSuspendUpdate( false );
        }
    }

private:
    HierarchyViewWidget* widget_;
    std::vector< std::pair<
            std::string,    /// name
            std::string     /// full path
        > > items_;
    std::string allStatesStr_;
    std::string itemStatesStr_;
    std::map< std::string, int > indexMap_;
};


////////////////////////////////////////////////////////////////////////////////
/// HierarchyViewKnob
////////////////////////////////////////////////////////////////////////////////

HierarchyViewKnob::HierarchyViewKnob( DD::Image::Knob_Closure* _kc, const char** _data, const char* _name, const char* _label )
    : DD::Image::Knob( _kc, _name, _label ), impl_( new HierarchyViewKnobImp( _data ) )
{
}

HierarchyViewKnob::~HierarchyViewKnob()
{
    if ( impl_ ) {
        delete impl_;
        impl_ = NULL;
    }
}

const char* HierarchyViewKnob::Class() const
{
    return "HierarchyViewKnob";
}

bool HierarchyViewKnob::not_default () const
{
    return impl_->not_default();
}

void HierarchyViewKnob::to_script ( std::ostream& _os, const DD::Image::OutputContext* _oc, bool _quote) const
{
    return impl_->to_script( _os, _oc, _quote );
}

bool HierarchyViewKnob::from_script( const char* _v )
{
    if ( _v ) {
        new_undo( "setValue" );
        impl_->from_script( _v );
        changed();
        return true;
    }
    return false;
}

void HierarchyViewKnob::store( DD::Image::StoreType _type, void* _data, DD::Image::Hash& _hash, const DD::Image::OutputContext& _oc )
{
    return impl_->store( _type, _data, _hash, _oc );
}

const char* HierarchyViewKnob::get_text( const DD::Image::OutputContext* _oc ) const
{
    return impl_->get_text( _oc );
}

#if kDDImageVersionInteger < 70000

WidgetPointer HierarchyViewKnob::make_widget()
{
    return impl_->make_widget( this );
}

#else

WidgetPointer HierarchyViewKnob::make_widget( const DD::Image::WidgetContext& _context )
{
    return impl_->make_widget( this, _context );
}

#endif

void HierarchyViewKnob::addCB( DD::Image::Knob::Callback _cb, void* _closure )
{
    return addCallback( _cb, _closure );
}

void HierarchyViewKnob::removeCB( DD::Image::Knob::Callback _cb, void* _closure )
{
    return removeCallback( _cb, _closure );
}

void HierarchyViewKnob::setHeader( const char* _text )
{
    if ( _text ) {
        impl_->setHeader( _text );
    }
}

void HierarchyViewKnob::setState( int _idx, int _v )
{
    if ( _idx >= 0 && static_cast< std::size_t >( _idx ) < impl_->statesSize() ) {
        new_undo( "setValue" );
        impl_->setState( _idx, _v );
        changed();
    }
}

int  HierarchyViewKnob::getState( int _idx ) const
{
    if ( _idx >= 0 && static_cast< std::size_t >( _idx ) < impl_->statesSize() ) {
        return impl_->getState( _idx );
    }
    return -1;
}


void HierarchyViewKnob::setItemState( int _idx, int _v )
{
    if ( _idx >= 0 && static_cast< std::size_t >( _idx ) < impl_->itemStatesSize() ) {
        new_undo( "setValue" );
        impl_->setItemState( _idx, _v );
        changed();
    }
}

int  HierarchyViewKnob::getItemState( int _idx ) const
{
    if ( _idx >= 0 && static_cast< std::size_t >( _idx ) < impl_->itemStatesSize() ) {
        return impl_->getItemState( _idx );
    }
    return -1;
}

void HierarchyViewKnob::clear()
{
    return impl_->clear();
}

void HierarchyViewKnob::reset( const char* const* _items, int _itemLen, char _sep, const char* _states, int _defaultState )
{
    new_undo( "setValue" );
    impl_->reset( _items, _itemLen, _sep, _states, _defaultState );
    changed();
}

////////////////////////////////////////////////////////////////////////////////
void* HierarchyViewKnob::createItemList()
{
    return ( void* )( new std::vector< const char* >() );
}

void  HierarchyViewKnob::destroyItemList( void* _itemList )
{
    std::vector< const char* >* itemList = ( std::vector< const char* >* )( _itemList );
    if ( itemList ) {
        for ( std::size_t idx( 0 ); idx < itemList->size(); ++idx ) {
            delete[] ( ( *itemList )[ idx ] );
        }
        delete itemList;
        _itemList = NULL;
    }
}

void  HierarchyViewKnob::appendItem( void* _itemList, const char* _item )
{
    if ( ! _itemList || ! _item ) {
        return;
    }

    std::vector< const char* >* itemList = ( std::vector< const char* >* )( _itemList );
    if ( itemList ) {
        std::size_t itemStrLen = ::strlen( _item );
#ifdef _WIN32
        char* item = new char[ itemStrLen + 1 ];
        ::strncpy_s( item, itemStrLen + 1, _item, itemStrLen );
        item[ itemStrLen ] = '\0';
#else
        char* item = new char[ itemStrLen ];
        ::strcpy( item, _item );
#endif
        itemList->push_back( item );
    }
}

void HierarchyViewKnob::reserve( void* _itemList, int _len )
{
    std::vector< const char* >* itemList = ( std::vector< const char* >* )( _itemList );
    if ( itemList ) {
        itemList->reserve( _len );
    }
}

int HierarchyViewKnob::getItemSize( void* _itemList )
{
    std::vector< const char* >* itemList = ( std::vector< const char* >* )( _itemList );
    if ( itemList ) {
        return ( int )( itemList->size() );
    }
    return 0;
}

const char* const* HierarchyViewKnob::getItemList( void* _itemList )
{
    std::vector< const char* >* itemList = ( std::vector< const char* >* )( _itemList );
    if ( itemList && ! itemList->empty() ) {
#if defined( _MSC_VER ) && ( _MSC_VER <= 1600 )
        return ( const char* const* )( &( *itemList )[ 0 ] );
#else
        return ( const char* const* )( itemList->data() );
#endif
    }
    return NULL;
}


