#include "AVWriterProcess.hpp"

#include <AvTranscoder/DatasStructures/Pixel.hpp>
#include <AvTranscoder/DatasStructures/AudioFrame.hpp>
#include <AvTranscoder/DatasStructures/DataStreamDesc.hpp>

#include <tuttle/plugin/exceptions.hpp>

namespace tuttle {
namespace plugin {
namespace av {
namespace writer {

template<class View>
AVWriterProcess<View>::AVWriterProcess( AVWriterPlugin& instance )
	: ImageGilFilterProcessor<View>( instance, eImageOrientationFromTopToBottom )
	, _plugin( instance )
	, _params( _plugin.getProcessParams() )
{
	this->setNoMultiThreading();
}

/**
 * @brief Function called by rendering thread each time a process must be done.
 * @param[in] procWindowRoW  Processing window in RoW
 */
template<class View>
void AVWriterProcess<View>::multiThreadProcessImages( const OfxRectI& procWindowRoW )
{
	using namespace terry;
	BOOST_ASSERT( procWindowRoW == this->_dstPixelRod );
	
	// Get image to encode
	avtranscoder::ImageDesc imageToEncodeDesc = _plugin._outputStreamVideo->getVideoDesc().getImageDesc();
	if( _plugin._imageToEncode == NULL )
	{
		_plugin._imageToEncode.reset( new avtranscoder::Image( imageToEncodeDesc ) );
	}
	
	// Get image image rgb
	if( _plugin._rgbImage == NULL )
	{
		// get pixel data of image rgb
		avtranscoder::Pixel oPixel;
		size_t pixelComponents = imageToEncodeDesc.getPixelDesc().getComponents(); // get this from gil view
		size_t pixelDepth = 8; // waiting for getMaxBitPerChannel() in avTranscoder
		oPixel.setBitsPerPixel( pixelDepth * pixelComponents );
		oPixel.setComponents( pixelComponents );
		oPixel.setColorComponents( avtranscoder::eComponentRgb );
		oPixel.setSubsampling( avtranscoder::eSubsamplingNone );
		oPixel.setAlpha( false );
		oPixel.setPlanar( false );
		
		avtranscoder::ImageDesc imageRGBDesc( imageToEncodeDesc );
		imageRGBDesc.setPixel( oPixel.findPixel() );
		_plugin._rgbImage.reset( new avtranscoder::Image( imageRGBDesc ) );
	}
	
	rgb8_image_t img ( this->_srcView.dimensions() );
	rgb8_view_t  vw  ( view( img ) );
	
	// Convert pixels to destination
	copy_and_convert_pixels( this->_srcView, this->_dstView );
	
	// Convert pixels in PIX_FMT_RGB24
	copy_and_convert_pixels( this->_srcView, vw );
	
	uint8_t* imageData = (uint8_t*)boost::gil::interleaved_view_get_raw_data( vw );
	
	// Set buffer of image rgb
	std::memcpy( _plugin._rgbImage->getPtr(), imageData, _plugin._rgbImage->getSize() );
	
	_plugin._colorTransform.convert( *_plugin._rgbImage, *_plugin._imageToEncode );
	
	// encode video
	avtranscoder::DataStream codedImage;
	if( _plugin._outputStreamVideo->encodeFrame( *_plugin._imageToEncode, codedImage ) )
	{
		_plugin._outputFile->wrap( codedImage, 0 );
	}
	
	// encode audio
	if( _plugin._initAudio )
	{
		avtranscoder::DataStream codedAudioFrame;
		avtranscoder::AudioFrame audioFrameSource( _plugin._inputAudioFile->getStream( _plugin._idAudioStream ).getAudioDesc().getFrameDesc() );
		avtranscoder::AudioFrame audioFrameToEncode( _plugin._outputStreamAudio->getAudioDesc().getFrameDesc() );

		_plugin._inputStreamAudio->readNextFrame( audioFrameSource );

		_plugin._audioTransform.convert( audioFrameSource, audioFrameToEncode );

		if( _plugin._outputStreamAudio->encodeFrame( audioFrameSource, codedAudioFrame ) )
		{
			_plugin._outputFile->wrap( codedAudioFrame, 0 );
		}
	}
}

}
}
}
}
