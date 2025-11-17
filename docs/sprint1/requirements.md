# Sprint 1 - Requirements Document
**Project Name:** Antipode: DayDreamNight  
**Document Version:** 1.0  
**Creation Date:** November 7, 2025  
**Team:** Group 12  

## 1. Project Overview

Antipode: DayDreamNight is a Qt-based global video social prototype application, inspired by the core concept of BeReal. The app connects two completely opposite regions on Earth by calculating the user's geographical **antipode**, allowing users to explore and share daily life videos from the other side of the world.

**Core Value Proposition:** "Discover Life on the Opposite Side of the Earth"

## 2. Functional Requirements

### 2.1 User Interface Framework

#### 2.1.1 Navigation Structure
- [ ] **Bottom Navigation Bar** (5 main pages)
  - [ ] Home - Video feed
  - [ ] Discover - Explore content
  - [ ] Capture - Core creation function
  - [ ] Messages - Social interaction
  - [ ] Profile - User management

#### 2.1.2 Page Detailed Requirements
- [ ] **Home Page**
  - [ ] Full-screen vertical video stream display
  - [ ] Vertical swipe to switch between videos
  - [ ] Floating action buttons (Like, Comment, Share, User Avatar)
  - [ ] Antipode location display component

- [ ] **Capture Page**
  - [ ] Camera preview interface
  - [ ] Front/back camera toggle
  - [ ] Record button with visual feedback
  - [ ] 10-second duration limit indicator

- [ ] **Profile Page**
  - [ ] User avatar, nickname, bio display
  - [ ] Video works grid layout
  - [ ] Edit profile entry
  - [ ] Settings options

### 2.2 Core Functional Requirements

#### 2.2.1 Geolocation Services
- [ ] **Automatic Antipode Calculation**
  - Input: User's current latitude and longitude coordinates
  - Process: `latitude → -latitude`, `longitude → longitude ± 180°`
  - Output: Antipode coordinates and region information
- [ ] **Location Permission Management**
  - [ ] Elegant permission request dialog
  - [ ] Graceful degradation for permission denial
  - [ ] Manual location selection alternative

#### 2.2.2 Video Features
- [ ] **Video Player**
  - [ ] Full-screen playback mode
  - [ ] Play/Pause control
  - [ ] Seek bar dragging
  - [ ] Volume adjustment
  - [ ] Auto-play next video

- [ ] **Video Recording**
  - [ ] 10-second duration limit
  - [ ] Recording preview interface
  - [ ] Basic video description editing
  - [ ] Automatic location tagging

#### 2.2.3 Social Interaction
- [ ] **User Engagement**
  - [ ] Like function (with animation feedback)
  - [ ] Comment viewing and posting
  - [ ] Share to other platforms (simulated implementation)
  - [ ] User follow system

## 3. Non-Functional Requirements

### 3.1 Internationalization Support
- [ ] **Multi-language Interface**
  - [ ] English (default)
  - [ ] Chinese (Simplified)
  - [ ] Language switching mechanism
- [ ] **Regional Adaptation**
  - [ ] Date and time format localization
  - [ ] Text direction support (LTR/RTL)

### 3.2 Accessibility Design
- [ ] **Visual Accessibility**
  - [ ] High contrast mode
  - [ ] Adjustable font size
  - [ ] Colorblind-friendly color scheme
- [ ] **Interaction Accessibility**
  - [ ] Complete keyboard navigation support
  - [ ] Screen reader compatibility
  - [ ] Clear focus indicators

### 3.3 Performance Requirements
- [ ] **Response Time**
  - [ ] Application launch time < 3 seconds
  - [ ] Video loading time < 2 seconds
  - [ ] Page switching response < 0.5 seconds
- [ ] **Stability**
  - [ ] No crashes on critical paths
  - [ ] Optimized memory usage
  - [ ] Network exception handling

### 3.4 Technical Constraints
- [ ] **Development Framework**: Qt 5.15.2+
- [ ] **Target Platform**: Windows/macOS (desktop first, mobile responsive)
- [ ] **Multimedia**: Qt Multimedia plugin
- [ ] **Data Storage**: Local file system + simulated data

## 4. Project Constraints

### 4.1 Time Constraints
- **Sprint 1 Cycle**: 1 week
- **Core Deliverables**: Runnable basic prototype
- **Focus**: Video playback + Basic navigation framework

### 4.2 Technical Constraints
- Use provided Tomeo starter code as foundation
- Prioritize functional stability over completeness
- Adopt simulation strategy for complex features

## 5. Acceptance Criteria

### 5.1 Functional Acceptance
- [ ] User can successfully launch the app and complete location initialization
- [ ] Video playback function is stable with basic playback controls
- [ ] Navigation framework is complete with all pages accessible
- [ ] Recording process front-end interaction is complete

### 5.2 Quality Acceptance
- [ ] Interface responds quickly without noticeable lag
- [ ] Proper error handling with no application crashes
- [ ] Clear code structure with complete comments
- [ ] Comprehensive documentation including README and deployment instructions

---

**Document Status:** ✅ Approved - For Sprint 1 Development  
**Next Review:** End of Sprint 1  
**Maintenance Owner:** XT (Documentation Team)
