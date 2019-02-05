//
//  InterfaceController.m
//  watch4 WatchKit Extension
//
//  Created by Bernhard Urban on 14.11.18.
//  Copyright © 2018 Bernhard Urban. All rights reserved.
//

#import "InterfaceController.h"
#import "runtime.h"

@interface InterfaceController ()

@end


@implementation InterfaceController

- (void)awakeWithContext:(id)context {
    [super awakeWithContext:context];

    // Configure interface objects here.
}

- (void)willActivate {
    // This method is called when watch view controller is about to be visible to user
    [super willActivate];
	dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0), ^{
			[self startRuntime];
		});
}

- (void)startRuntime {
	mono_ios_runtime_init ();
}

- (void)didDeactivate {
    // This method is called when watch view controller is no longer visible
    [super didDeactivate];
}

@end



