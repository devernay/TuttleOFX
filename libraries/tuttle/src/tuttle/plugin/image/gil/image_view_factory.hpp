#ifndef _TUTTLE_PLUGIN_GIL_IMAGEVIEWFACTORY_HPP_
#define _TUTTLE_PLUGIN_GIL_IMAGEVIEWFACTORY_HPP_

// code which need to be included in boost::gil
// to allow writting:
// channel_view_type<red_t>(myview)
// instead of:
// kth_channel_view_type<0>(myview)
namespace boost {
namespace gil {
template <typename Channel, typename View>
struct channel_type_to_index
{
	static const int value = gil::detail::type_to_index<
	    typename color_space_type<View>::type,                       // color (mpl::vector)
	    Channel                       // channel type
	    >::type::value;                     //< index of the channel in the color (mpl::vector)
};

template <typename Channel, typename View>
struct channel_view_type : public kth_channel_view_type<channel_type_to_index<Channel, View>::value, View>
{
	static const int index = channel_type_to_index<Channel, View>::value;
	typedef kth_channel_view_type<index, View> parent_t;
	typedef typename parent_t::type type;
	static type make( const View& src ) { return parent_t::make( src ); }
};

/// \ingroup ImageViewTransformationsKthChannel
template <typename Channel, typename View>
typename channel_view_type<Channel, View>::type channel_view( const View& src )
{
	return channel_view_type<Channel, View>::make( src );
}

}
}

#endif
