# Contributing to Crest

Thank you for your interest in contributing to Crest! This document provides guidelines and instructions for contributing.

## Code of Conduct

Be respectful and inclusive. We welcome contributions from everyone.

## How to Contribute

### Reporting Bugs

1. Check if the bug has already been reported in [Issues](https://github.com/muhammad-fiaz/crest/issues)
2. If not, create a new issue with:
   - Clear title and description
   - Steps to reproduce
   - Expected vs actual behavior
   - System information (OS, compiler version, etc.)
   - Code samples if applicable

### Suggesting Features

1. Check existing issues and discussions
2. Create a new issue with:
   - Clear description of the feature
   - Use cases and benefits
   - Possible implementation approach

### Pull Requests

1. Fork the repository
2. Create a feature branch (`git checkout -b feature/amazing-feature`)
3. Make your changes
4. Add tests for new functionality
5. Ensure all tests pass (`xmake build crest_tests && xmake run crest_tests`)
6. Update documentation
7. Commit your changes (`git commit -m 'Add amazing feature'`)
8. Push to the branch (`git push origin feature/amazing-feature`)
9. Open a Pull Request

## Development Setup

### Prerequisites

- C++20 compatible compiler (GCC 10+, Clang 12+, MSVC 2019+)
- C17 compatible compiler
- xmake 2.8.0+
- Git

### Building from Source

```bash
git clone https://github.com/muhammad-fiaz/crest.git
cd crest
xmake config -m debug
xmake build
```

### Running Tests

```bash
xmake build crest_tests
xmake run crest_tests
```

### Building Examples

```bash
xmake build crest_example_cpp
xmake build crest_example_c
xmake run crest_example_cpp
```

## Coding Standards

### C++ Code

- Use C++20 features where appropriate
- Follow modern C++ best practices
- Use RAII for resource management
- Prefer `std::string` over C-style strings
- Use `const` and `constexpr` where applicable
- Follow the existing code style

### C Code

- Use C17 standard
- Follow existing naming conventions
- Document all public APIs
- Check for NULL pointers
- Free allocated memory properly

### General Guidelines

- Write clear, self-documenting code
- Add comments for complex logic
- Keep functions focused and small
- Avoid premature optimization
- Write tests for new features
- Update documentation

## Documentation

- Update relevant `.md` files in `docs/`
- Add docstrings to new functions
- Include code examples
- Update README.md if needed

## Testing

- Write unit tests for new features
- Ensure existing tests pass
- Test on multiple platforms if possible
- Include edge cases in tests

## Commit Messages

Use clear, descriptive commit messages:

```
Add feature: Brief description

Detailed explanation of what changed and why.
Include any relevant issue numbers.

Fixes #123
```

## Review Process

1. Maintainers will review your PR
2. Address any requested changes
3. Once approved, your PR will be merged

## Questions?

Feel free to:
- Open an issue for questions
- Email: contact@muhammadfiaz.com
- Start a discussion on GitHub

## License

By contributing, you agree that your contributions will be licensed under the MIT License.

Thank you for contributing to Crest! 🌊
