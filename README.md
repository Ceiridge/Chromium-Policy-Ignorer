# Chromium Policy Ignorer
Sometimes, your boss wants to block you from opening dev tools. What a jerk! Using this tool to *jailbreak* your Chromium browser (Chrome, Edge, Brave, etc.) is possible, but should only be done with allowance by your organisation.

Ignore Chromium's policy registry keys by hooking into `RegQueryInfoKeyW` (WINAPI function), so that it returns zero results when querying the policy keys.

## Copyright
Chromium-Policy-Ignorer is released into the public domain according to the GPL 3.0 license by the copyright holders.

Disclaimer: This repository and the used names "Chrome", "Chromium", "Edge" in this project are not affiliated with or endorsed by Google LLC, Microsoft, The Chromium Project, Microsoft Edge, Google Chrome or other third parties. This repository and the used names "Chrome", "Chromium", "Edge" are also not affiliated with any existing trademarks.

**No code was copied** or used from any other browser in this repository. Chromium is licensed under the open source BSD License.

This repository does not infringe any copyright of proprietary browsers, as it only patches bytes on the end user's computer, without having any copyright-protected code or text included in this repository.
