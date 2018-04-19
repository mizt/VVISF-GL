#import "ISFFuncTestAppDelegate.h"




using namespace VVISF;
using namespace VVGL;




@implementation ISFFuncTestAppDelegate


- (id) init	{
	self = [super init];
	if (self != nil)	{
		sharedContext = nullptr;
		scene = nullptr;
	}
	return self;
}
- (void)applicationDidFinishLaunching:(NSNotification *)aNotification {
	[self loadBackendFromDefaults];
	//	make the displaylink, which will drive rendering
	CVReturn				err = kCVReturnSuccess;
	CGOpenGLDisplayMask		totalDisplayMask = 0;
	GLint					virtualScreen = 0;
	GLint					displayMask = 0;
	NSOpenGLPixelFormat		*format = [[[NSOpenGLPixelFormat alloc] initWithCGLPixelFormatObj:CreateCompatibilityGLPixelFormat()] autorelease];
	
	for (virtualScreen=0; virtualScreen<[format numberOfVirtualScreens]; ++virtualScreen)	{
		[format getValues:&displayMask forAttribute:NSOpenGLPFAScreenMask forVirtualScreen:virtualScreen];
		totalDisplayMask |= displayMask;
	}
	err = CVDisplayLinkCreateWithOpenGLDisplayMask(totalDisplayMask, &displayLink);
	if (err)	{
		NSLog(@"\t\terr %d creating display link in %s",err,__func__);
		displayLink = NULL;
	}
	else	{
		CVDisplayLinkSetOutputCallback(displayLink, displayLinkCallback, self);
		CVDisplayLinkStart(displayLink);
	}
}

- (void) loadBackendFromDefaults	{
	//NSLog(@"%s",__func__);
	@synchronized (self)	{
		sharedContext = nullptr;
		scene = nullptr;
	
		NSUserDefaults		*def = [NSUserDefaults standardUserDefaults];
		NSNumber			*tmpNum = [def objectForKey:@"glVers"];
		if (tmpNum == nil)
			tmpNum = [NSNumber numberWithInteger:GLVersion_2];
	
		switch ([tmpNum intValue])	{
		case GLVersion_2:
			sharedContext = make_shared<GLContext>(nullptr, CreateCompatibilityGLPixelFormat());
			[glVersPUB selectItemWithTag:2];
			break;
		default:
			sharedContext = make_shared<GLContext>(nullptr, CreateGL4PixelFormat());
			[glVersPUB selectItemWithTag:4];
			break;
		}
	
		CreateGlobalBufferPool(sharedContext);
	
		scene = make_shared<ISFScene>();
	
		[bufferView setSharedGLContext:sharedContext];
	
		NSString		*tmpStr = [[NSBundle mainBundle] pathForResource:@"CellMod" ofType:@"fs"];
		if (tmpStr != nil)	{
			//string			path([tmpStr UTF8String]);
			scene->useFile(string([tmpStr UTF8String]));
		}
	}
}

- (IBAction) glVersPUBUsed:(id)sender	{
	NSMenuItem		*selItem = [glVersPUB selectedItem];
	if (selItem == nil)
		return;
	
	NSNumber		*newVersDefaultsVal = nil;
	switch ([selItem tag])	{
	case 2:
		newVersDefaultsVal = [NSNumber numberWithInteger:GLVersion_2];
		break;
	case 4:
		newVersDefaultsVal = [NSNumber numberWithInteger:GLVersion_4];
		break;
	}
	
	if (newVersDefaultsVal == nil)
		return;
	
	NSUserDefaults	*def = [NSUserDefaults standardUserDefaults];
	[def setObject:newVersDefaultsVal forKey:@"glVers"];
	[def synchronize];
	
	[self loadBackendFromDefaults];
}

//	this method is called from the displaylink callback
- (void) renderCallback	{
	//NSLog(@"%s",__func__);
	@synchronized (self)	{
		if (scene == nullptr)
			return;
		NSRect				viewFrame = [bufferView frame];
		GLBufferRef		tmpBuffer = scene->createAndRenderABuffer(VVGL::Size(viewFrame.size.width, viewFrame.size.height));
		[bufferView drawBuffer:tmpBuffer];
		//	tell the buffer pool to do its housekeeping (releases any "old" resources in the pool that have been sticking around for a while)
		GetGlobalBufferPool()->housekeeping();
	}
}


@end




CVReturn displayLinkCallback(CVDisplayLinkRef displayLink, 
	const CVTimeStamp *inNow, 
	const CVTimeStamp *inOutputTime, 
	CVOptionFlags flagsIn, 
	CVOptionFlags *flagsOut, 
	void *displayLinkContext)
{
	NSAutoreleasePool		*pool =[[NSAutoreleasePool alloc] init];
	[(ISFFuncTestAppDelegate *)displayLinkContext renderCallback];
	[pool release];
	return kCVReturnSuccess;
}
