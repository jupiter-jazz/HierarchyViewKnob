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

#ifndef HIERARCHY_VIEW_WIDGET_H
#define HIERARCHY_VIEW_WIDGET_H

#include <DDImage/Knob.h>

#include <QtCore/QObject>
#include <QtGui>
#include <QTreeWidget>
#include <QHash>
#include <QPair>
#include <QVector>

class HierarchyViewKnob;
class HierarchyViewWidget : public QTreeWidget
{
    Q_OBJECT

public:
    HierarchyViewWidget( HierarchyViewKnob* _knob );
    virtual ~HierarchyViewWidget();

    void update();
    void destroy();
    static int WidgetCallback( void* _closure, DD::Image::Knob::CallbackReason _reason );

    /// get and set absolute indices, a.k.a indices after the hierarchy being flattened
    int  getAbsIndex( QTreeWidgetItem* _item ) const;
    void setAbsIndex( QTreeWidgetItem* _item, int _idx );

    /// get the original indices, a.k.a indices before the hierarchy being flattened
    const QVector< QPair< QString, QTreeWidgetItem* > >& itemIndices() const;
    QVector< QPair< QString, QTreeWidgetItem* > >& itemIndices();

    /// traverse all parent items to get the state
    bool parentItemState( QTreeWidgetItem* parentItem ) const;

    /// get and set updating state, provided for convenience to indicate the state when widget update
    bool getSuspendUpdate() const;
    void setSuspendUpdate( bool _suspendUpdate );

public Q_SLOTS:
    void valueChanged( QTreeWidgetItem* _item , int _column );

protected:
    virtual void wheelEvent( QWheelEvent* _event );

private:
    /// the knob which this widget belongs to
    HierarchyViewKnob* knob_;
    /// absolute indices
    QHash< QTreeWidgetItem*, int > absIdxHash_;
    /// original indices
    QVector< QPair< QString, QTreeWidgetItem* > > idxArray_;
    /// updating flag
    bool suspendUpdate_;
};

#endif

