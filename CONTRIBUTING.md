# Contributing to The Amphora Project

The Amphora Project is happy to accept contributions!

These will typically come in one of two forms:

1. Community modded forks
2. Core engine features/enhancements

The first category might include specific engine builds optimized for particular use cases (e.g. a deterministic build for speed-running, or a build with reduced memory footprint for mobile devices).
The core engine should be thought of as a reference API implementation, and so while we wholeheartedly encourage whatever cool ideas the community may have, these should be treated as separate projects dependent on upstream, and not features that should be merged back into the core project.
These forks may optionally be listed in the community forks list at [https://amphora-project.org/community-forks](https://amphora-project.org/community-forks). (This is currently a work in progress and is not yet live)

The second category encompasses contributions that *are* meant to be merged back into the core project, such as bug fixes, optimizations, or features considered foundational to the function of the engine, and are the focus of this document.

## How to Contribute

The repository, [amphora-engine/amphora-engine](https://github.com/amphora-engine/amphora-engine) is the canonical repository for the Amphora project.
This repository should be taken as the single source of truth for the project, you will not see any other official branches or forks.
The repository should be treated as a linear history with a single branch, `main`.

Development primarily happens on the [engine-devel@amphora-project.org](mailto:engine-devel@amphora-project.org) mailing list.
This is currently a work in progress and is not yet live, so until it is, patches can be sent directly to [caleb@amphora-project.org](mailto:caleb@amphora-project.org).
This is where patches should be submitted for review by a maintainer.
Patches should be formatted using `git format-patch` and then sent to the mailing list with `git send-email`.
Trivial pull requests (e.g. typo fixes) may be opened against `main` without discussion and may be accepted at the discretion of a maintainer.

The intention behind these decisions is to encourage focused contributions and development.
Discussion and patch review should be linear, referenceable, and archivable; and should avoid the sprawl and confusion of GitHub branches and pull requests.
The content of patches should also be kept to a minimum, and should be focused on the specific change being made.
Many small patches are preferred to fewer large patches, and patches should avoid directly depending on each other (If two patches are submitted together and the second depends on the first, consider whether they would be better served as a single patch).

### Note to New Contributors

We understand that these guidelines are strict; they exist to ensure the direction of the project and the quality of contributions.
If you are interested in and serious about learning and contributing but are unfamiliar with the processes, feel free to reach out to the mailing list or a maintainer and we'll be happy to help you get your footing.
The Amphora Project is not a place where elitism is tolerated.

## Code of Conduct

The Amphora Project operates as a high-trust community of professionals.
Contributors are expected to focus on code, treat others with respect, and handle themselves as adults.

Maintainers will not intervene in disputes regarding technical disagreement, blunt feedback, or criticism of code quality.
That said, behavior that actively undermines the community, such as harassment, personal attacks, persistent off-topic discussion, or elitism, will not be tolerated.

For example, statements such as "this patch is poor quality" or "this feature is out of scope" are considered normal parts of the technical review process, even if they are delivered bluntly.
Statements that attack a contributor based on personal characteristics, or that continually derail technical discussion, are not acceptable.

Violations of these guidelines will be handled as follows:
1. A first offense will result in the offending member being asked to alter their behavior.
To err is human, and our goal is to create a high-trust environment and to assume the best of intentions by default.
2. A second offense will result in the guidelines being firmly reiterated.
This is a more serious warning, and based on the nature of the violation, further consequences may be determined as necessary.
3. A third offense will result in the offending member being banned from the community.
This action is final, as continued offenses demonstrate a lack of respect for the community, and this is actively incompatible with the high-trust environment we aim to create.

Violations should be referred to a maintainer for escalation or resolution.

## Coding Style

The Amphora Project generally follows C89/C90 coding standards, with a handful of features from C99 being allowed.
Specifically, these include compound literals, designated initializers, long long, stdint, and stdbool.
The biggest implication of this is that we do not allow declarations to be mixed with code, though declarations may be initialized.

The project uses Allman style braces, with indentation being a tab (not spaces).
Function return types should be on a separate line from the function name.
In practice, a function would look like this:
```c
int
foo(void)
{
	return 0;
}
```

Functions that return a value that is unused should be cast to `void` to ensure that we are explicit in our intentions.
As an example:
```c
/* This would be rejected */
printf("Hello, world!\n");

/* This would be accepted */
(void)printf("Hello, world!\n");
```

All function names should be prefixed with `Amphora_` and should follow the PascalCase convention from there.
Public functions must also be suffixed with their API version in the format `VX` with X being the API version number.

Once an API version is frozen, all functions belonging to that version cannot change their signatures.
Improvements may be made to their implementation, but the API is not allowed to change.
If the API must change, a new function must be written with the new API version number suffix and a different signature.
Functions must not be removed once they have been released.

When a new API version development cycle begins, it will be announced on the mailing list, and all new public functions contributed during that period must be suffixed with the current API version number.
These development cycles are determined by the maintainers.

## Project Structure

Each engine component is made up of a single .c file and two headers. The .c file contains the implementation and should be found in `/src`.
The headers are included in `/include` for the component's public header, and `/include/internal` for the component's internal header.

The internal header must include the public header.
The public header declares functions and defines structures that are part of the component's API and are meant to be called from consuming game code.
The internal header declares functions and defines structures that are allowed to be called from other components, but not from game code.
Private functions and structures that can only be called or used from within the component are declared in the component's implementation file.

The implementation file is structured from top to bottom with prototypes for private functions, followed by static state variables, next public functions, then internal functions, and finally private functions.
There are a few special cases to this rule, specifically concerning functions that are only meant to be called from within a single specific other component, or concerning registration of game data supplied by the consumer.
These cases should be used as sparingly as possible, must be documented as they occur and must not be confusing to follow.

## Content Policy

- Plagiarism is absolutely unacceptable.
If you submit a patch, you are solely responsible for ensuring that it does not contain plagiarized content.
Failure to do so will result in your patch being rejected/removed, and may result in future patches not being considered.
- By submitting a patch to the mailing list, you are attesting that you have a complete and total understanding of the patch being proposed.
You may be expected to engage in discussion regarding the patch, and if it is apparent that you do not understand the code that you have submitted, the patch will be rejected.

### AI Specific Stipulations

The Amphora Project understands and appreciates the value that responsible use of AI tools can offer.
Generally, it is better to use AI tools to assist in debugging and optimization than to use them to generate content.
With that in mind though, AI-generated content can be considered for inclusion under the policies above.
Disclosures are not required.
If a patch is of high quality and adheres to the project guidelines and policies, the method by which it was written is irrelevant.
