#ifndef _TUTTLE_PLUGIN_BASICKEYER_PLUGIN_HPP_
#define _TUTTLE_PLUGIN_BASICKEYER_PLUGIN_HPP_

#include "BasicKeyerDefinitions.hpp"

#include <tuttle/common/utils/global.hpp>
#include <ofxsImageEffect.h>
#include <boost/gil/gil_all.hpp>

namespace tuttle {
namespace plugin {
namespace basicKeyer {

template<class View>
struct BasicKeyerProcessParams
{
	typedef typename View::value_type Pixel;
	typedef boost::gil::point2<double> Point2;
	Point2 _cornerA;
	Point2 _cornerB;
	std::vector<Point2> _points;
	std::vector<boost::gil::rgba32f_pixel_t> _colors;
};

/**
 * @brief BasicKeyer plugin
 */
class BasicKeyerPlugin : public OFX::ImageEffect
{
public:
	typedef boost::gil::point2<double> Point2;

public:
	BasicKeyerPlugin( OfxImageEffectHandle handle );

public:
	void render( const OFX::RenderArguments& args );
	void changedParam( const OFX::InstanceChangedArgs& args, const std::string& paramName );

	template<class View>
	BasicKeyerProcessParams<View> getProcessParams() const;

public:
	typedef std::vector<OFX::Double2DParam*> Double2DParamVector;
	typedef std::vector<OFX::RGBAParam*> RGBAParamVector;

	// do not need to delete these, the ImageEffect is managing them for us
	OFX::Clip* _clipSrc;       ///< Source image clip
	OFX::Clip* _clipDst;       ///< Destination image clip
	Double2DParamVector _paramPoints;
	RGBAParamVector _paramColors;
	OFX::ChoiceParam* _paramMode;
	OFX::IntParam* _paramNbPoints;

private:
	OFX::InstanceChangedArgs _instanceChangedArgs;
};

}
}
}

#endif
