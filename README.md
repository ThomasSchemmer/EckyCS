<!--
Template taken from 
https://github.com/ratatui/templates
-->

<div align="center">

  <img src="Logo.png" alt="logo" width="200" height="auto" />
  <h1>EckyCS</h1>
  
  <p>
    An icky Entity Component System - or: experiments in writing an ECS
  </p>

  
<!-- Badges -->
<p>
  <a href="https://github.com/ThomasSchemmer/EckyCS/graphs/contributors">
    <img src="https://img.shields.io/github/contributors/ThomasSchemmer/EckyCS" alt="contributors" />
  </a>
  <a href="">
    <img src="https://img.shields.io/github/last-commit/ThomasSchemmer/EckyCS" alt="last update" />
  </a>
  <a href="https://github.com/ThomasSchemmer/EckyCS/blob/master/LICENSE">
    <img src="https://img.shields.io/github/license/ThomasSchemmer/EckyCS.svg" alt="license" />
  </a>
</p>
   
<h4>
    <a href="https://github.com/ThomasSchemmer/EckyCS/tree/master/docs">Documentation</a>
  <span> · </span>
    <a href="https://github.com/ThomasSchemmer/EckyCS/issues/">Report Bug</a>
  <span> · </span>
    <a href="https://github.com/ThomasSchemmer/EckyCS/issues/">Request Feature</a>
  </h4>
</div>

<br />

<!-- Table of Contents -->
# :notebook_with_decorative_cover: Table of Contents

- [About the Project](#star2-about-the-project)
- [Roadmap](#compass-roadmap)
- [License](#warning-license)
- [Acknowledgements](#gem-acknowledgements)
  

<!-- About the Project -->
## About the Project

<div align="left"> 
  I was programming a jump-and-run, but the movement felt sluggish. So I pivoted to adding much more enemies to hide it :)<br>
  The plan is to have ~50k units on the screen in a fantasy ARPG, but for now I'm writing the underlying system
</div>

<!-- Roadmap -->
## :compass: Roadmap

* [x] SparseSet implementation
* [x] ComponentGroups and Queries
* [ ] BVH Tree generation for location queries
* [ ] Improved 
* [ ] Gameplay effect integration
* [ ] Example spells


<!-- License -->
## :warning: License

Distributed under the MIT License. See LICENSE.txt for more information.

<!-- Acknowledgments -->
## :gem: Acknowledgements

The ECS base code is heavily influenced by the great writeup at [skypjack.github.io](https://skypjack.github.io/2020-03-14-ecs-baf-part-8/) <br>
as well as [cient.dev](https://www.youtube.com/@cient_dev) videos on youtube.<br>
The burst compiler used for jobs is provided by Unity in their engine
