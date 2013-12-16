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

#include "DDImage/NukeWrapper.h"
#include "DDImage/PixelIop.h"
#include "DDImage/Row.h"
#include "DDImage/Knobs.h"

#include "HierarchyViewKnob.h"

#include <sstream>

using namespace DD::Image;

/// a global list, just for example
const char* items[] = {
    "/root/body/left_arm/finger1",
    "/root/body/left_arm/finger2",
    "/root/body/left_arm/finger3",
    "/root/body/right_arm/finger1",
    "/root/body/right_arm/finger2",
    "/root/body/right_arm/finger3",
    "/root/legs/left_foot",
    "/root/legs/right_foot",
    "/other/clothes/left_foot",
    "/other/clothes/left_leg",
    "/other/clothes/right_foot",
    "/other/clothes/right_leg",
    "/other/shoes/left_foot",
    "/other/shoes/right_foot",
};

class HierarchyViewKnobExample : public PixelIop
{
private:
    const char* filename_;
    const char* v_;
public:
    void in_channels(int input, ChannelSet& mask) const;
    HierarchyViewKnobExample(Node* node) : PixelIop(node), filename_(""), v_("") {}
    bool pass_transform() const { return true; }
    void pixel_engine(const Row &in, int y, int x, int r, ChannelMask, Row & out);
    virtual void knobs(Knob_Callback);
    int knob_changed( Knob* k );
    static const Iop::Description d;
    const char* Class() const { return d.name; }
    const char* node_help() const { return "Example Op to demonstrate how to use libHierarchyViewKnob."; }
    void _validate(bool);
    void resetGeoScene();
};

void HierarchyViewKnobExample::_validate(bool for_real)
{
    copy_info();
    set_out_channels(Mask_None);

    /// retrieve and show the states
    HierarchyViewKnob* hk = ( HierarchyViewKnob* )( knob( "test" ) );
    if ( hk ) {
        for ( int idx( 0 ); idx < 14; ++idx ) {
            std::cerr << "item " << items[idx] << " state is " << hk->getItemState( idx ) << std::endl;
        }
    }
}

void HierarchyViewKnobExample::in_channels(int input, ChannelSet& mask) const
{
}

void HierarchyViewKnobExample::pixel_engine(const Row& in, int y, int x, int r, ChannelMask channels, Row& out)
{
}

void HierarchyViewKnobExample::knobs(Knob_Callback f)
{
    File_knob( f, &filename_, "file" );
    /// a custom knob with custom data
    CustomKnob1( HierarchyViewKnob, f, &v_, "test" );
    //SetFlags( f, Knob::SAVE_MENU | Knob::ALWAYS_SAVE | Knob::KNOB_CHANGED_RECURSIVE);
    SetFlags( f, Knob::EARLY_STORE | Knob::SAVE_MENU | Knob::ALWAYS_SAVE);
}

void HierarchyViewKnobExample::resetGeoScene()
{
    Knob* k = knob( "file" );
    HierarchyViewKnob* hk = ( HierarchyViewKnob* )( knob( "test" ) );
    if ( hk ) {
        hk->setHeader( k->get_text() );

        if ( k->get_text() && ::strlen( k->get_text() ) ) {

            const char* states = knob( "test" )->get_text();
            /// use 'ItemList' structure to create a dynamic array
            /// in concrete case, the array should be build from a 'Reader' like
            /// function
            /// here is just a demo usage about 'ItemList'
            HierarchyViewKnob::ItemList itm;
            for ( int idx( 0 ); idx < 14; ++idx ) {
                HierarchyViewKnob::appendItem( itm.data, items[idx] );
            }
            hk->reset( HierarchyViewKnob::getItemList( itm.data ), HierarchyViewKnob::getItemSize( itm.data ), '/', states, 1 );
        } else {
            /// clear the list
            hk->clear();
        }
    }
}

int HierarchyViewKnobExample::knob_changed( Knob* k )
{
    if ( k->is( "file" ) ) {
        resetGeoScene();
        return 1;
    }

    if ( k == &Knob::showPanel ) {
        resetGeoScene();
        return 1;
    }
    return PixelIop::knob_changed( k );
}

static Iop* build(Node* node) { return new NukeWrapper(new HierarchyViewKnobExample(node)); }
const Iop::Description HierarchyViewKnobExample::d("HierarchyViewKnobExample", "HierarchyViewKnobExample", build);
