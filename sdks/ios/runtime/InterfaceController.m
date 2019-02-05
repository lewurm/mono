//
//  InterfaceController.m
//  watch4 WatchKit Extension
//
//  Created by Bernhard Urban on 14.11.18.
//  Copyright © 2018 Bernhard Urban. All rights reserved.
//

#import "InterfaceController.h"
void *mono_jit_init_version      (const char *root_domain_name, const char *runtime_version);

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
    mono_jit_init_version ("Mono.ios", "mobile");
}

- (void)didDeactivate {
    // This method is called when watch view controller is no longer visible
    [super didDeactivate];
}

@end



