---
name: Bug Report
description: Report a bug or issue with Crest
title: "[BUG] "
labels: ["bug", "triage"]
assignees: []
body:
  - type: markdown
    attributes:
      value: |
        ## Bug Report
        
        Thank you for reporting a bug! Please fill out the information below to help us investigate and fix the issue.

  - type: textarea
    id: description
    attributes:
      label: Describe the Bug
      description: A clear and concise description of what the bug is.
      placeholder: "Describe what happened, what you expected to happen, and any error messages you received."
    validations:
      required: true

  - type: textarea
    id: reproduction
    attributes:
      label: Steps to Reproduce
      description: Provide step-by-step instructions to reproduce the issue.
      placeholder: |
        1. Go to '...'
        2. Click on '....'
        3. Scroll down to '....'
        4. See error
    validations:
      required: true

  - type: textarea
    id: expected-behavior
    attributes:
      label: Expected Behavior
      description: What should have happened instead?
      placeholder: "Describe what you expected to happen."
    validations:
      required: true

  - type: dropdown
    id: platform
    attributes:
      label: Platform
      description: What platform are you running on?
      options:
        - Windows
        - Linux
        - macOS
        - Other (please specify)
    validations:
      required: true

  - type: input
    id: version
    attributes:
      label: Crest Version
      description: What version of Crest are you using?
      placeholder: "e.g., v1.0.0, latest commit hash"
    validations:
      required: true

  - type: input
    id: compiler
    attributes:
      label: Compiler & Version
      description: What compiler and version are you using?
      placeholder: "e.g., GCC 9.3.0, Clang 11.0.0, MSVC 2019"
    validations:
      required: true

  - type: textarea
    id: additional-context
    attributes:
      label: Additional Context
      description: Add any other context about the problem here.
      placeholder: "Screenshots, code snippets, build logs, etc."

  - type: checkboxes
    id: checklist
    attributes:
      label: Checklist
      description: Please check the following before submitting
      options:
        - label: I have searched for similar issues and couldn't find any
          required: true
        - label: I have included a minimal code example that reproduces the issue
          required: true
        - label: I have checked that this issue is not a duplicate
          required: true