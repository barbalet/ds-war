# Urban War: Open Source Simulation

## Built with the ApeSDK

### Background

Urban War came out of a series of developments. In the early 2000s, the ApeSDK was required to run Windows, Linux, and Mac. The solution to this was the Generic Platform Interface or GPI that was used to run a strip-down simulation on Mac, Windows, and Linux with a very simple single window interface. This was used in the development of the war simulation, primarily because it was the easiest interface to use and also run the main simulation.

The war simulation was developed when Tom Barbalet lived in England as there was a need for an open source war-game simulator as there was major promotion of a simulation on a TV show, Time Commanders. At the time as there was no open source war gaming simulation, Tom Barbalet developed the war simulation as a means of providing ideas in pitched battle in simulation terms that could be easily implemented.

Earlier, Barbalet had also developed the urban simulation around an urban terrorist simulation, where he got a grant from the Commonwealth Science and Industry Research Organization (CSIRO) in Australia, but never took the grant because he wanted to maintain the open source intellectual property so it could be used by others. Thus, the two simulations came out of independent work with the ApeSDK.

The war simulation and the urban simulation continued development independent underneath the GPI on it's base because they were providing different functionality and different interesting simulations. The urban simulation was used to simulate tens of thousands of city blocks and provided a detailed possibility of what the interior of buildings would look like when you just knew the exterior of buildings.

### Use in London 1940

In 2019, Barbalet started a simulation of London in 1940 as part of the Sim Sea Lion development. This required large scale maps, taking the PNG files that had been purchased for from the Library of Scotland and making them into JSON files that the urban simulation could absorb and render urban maps with. There were many iterations of this process, but it was necessary to create a map editor as the JSON files that were produced through the automated map analysis produced rough buildings.

A map editor and a drawing interface for creating map outputs of the JSON that was created by the map analysis in Sim Sea Lion was also added.

Urban War contains the urban simulation and the war simulation. Both can be compiled independently, and the ability to keep these things distinct currently is relatively important because the urban simulation contains information for simulating London in 1940.

Any kind of military activity, any simulation of the home guard and any opposing force in the case of Operation Sea Lion as a possibility in the Second World War would require probably the war simulation integration. Not quite there yet. The process of developing London in 1940 to the best possible resolution. Urban War is almost a subset of Sim Sea Lion now. The intertwining of all these projects is useful. Occasionally the urban simulation is needed for testing agent models where the actual data which is available through Sim Sea Lion is still relatively raw.

### The Internals of Unknown Buildings

The internals of buildings are not immediately accessible with the maps that are presented. The urban simulation is based on suburbs of thirty years ago, which haven't changed in terms of how the interiors of houses can be rendered from those suburbs, there have been a wide variety of changes in Western housing styles. They haven't caused the urban simulation to be that erroneous currently.

There are certain methods that are used in buildings found in London in 1940, which are slightly different, therefore the urban simulation requires minor tweaking in order to get it to render the possibilities of what the interiors of buildings were in London and in 1940. There were a number of ideas within this code that are still incredibly powerful and still incredibly useful. Windows, doors and the movement between rooms and every room needs to be accessible. Seemingly random created set of interiors. Practically, they're algorithmically created in a way where the rooms have important functions.

This is what you find with Urban War, it's a completely open source means of developing and exploring the ideas described.

### Cross Platform Development vs Functionality

The linking functionality of applications run on Mac, Windows, Linux, some of that has been temporarily lost through the maintenance of primarily Mac-based simulation. All of that is still there functionally, it just requires upkeep and updates accordingly.

There are generic platform interfaces that are used between both the war simulation and the urban simulation shouldn't be forgotten. So enter into this the new developer that is looking for something to add to either of these simulations, or to add to the map editor and visualization techniques or movement or editing techniques. There are many components to these things that still require active work.

### Large CI/CD integration with Gitlab

These elements are currently servicing the command-line development, which is still in a relatively early phase, the need for this code to be active and developed upon is relatively great. Urban War is now also backed with a large CI/CD integration.

The JSON files that are presented for the Sim Sea Lion work are relatively dynamic and can be changed easily through the map analysis software. Urban War now functions as the map verifying software. All the outputted JSON from Sim Sea Lion needs to be completely compatible with the Urban War interface.

Over time, the Urban War interface has changed, progressively, to allow for these new relatively basic Sim Sea Lion maps to come in, but also to still allow for the long-standing addition of doors, windows and internal rooms. These concepts are always required when creating buildings in simulation space.

### Getting Involved

If you want to get a sense of Urban War, the best way to do it is to download the source code from GitLab and build the source code ideally on Xcode on the Mac, if that is your preferred development platform. This is always tested, always running, and always reliable, you'll probably need the latest version of Xcode because the simulation has continued development the Xcode versions have increased.
You should be able to run everything that's being described here. Not just the war simulation, the urban simulation, but also the map editing software, the drawing output, and get a sense of what these bits of software are together, and then you can begin to modify.

For example, aspects of the agent modeling in the urban simulation, have follow-on effects through continued development of Sim Sea Lion. The linking of all the code together means that any commits that you make to test will immediately be tested against the source code, against the existing maps of interest and a variety of outputs in PNG format. You can see in real time what your changes have done to various aspects of the software within these combined simulations.

Please get in contact with Tom Barbalet as you make changes and make development decisions. There's a lot of background with the software, but also there are a number of outstanding things that still need to be worked on here. It's a perfect open source project if you're looking for something that's existing, robust and gives immediate feedback. It also enables you to make real progress in the development that is ongoing.

All email correspondence can be directed to Tom Barbalet barbalet at gmail dot com.