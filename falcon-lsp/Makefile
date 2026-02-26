.PHONY: install install-dev test lint clean

install:
	pip install .

install-dev:
	pip install -e ".[dev]"

test:
	pytest tests/ -v

lint:
	flake8 falcon_lsp/ tests/

clean:
	rm -rf build/ dist/ *.egg-info __pycache__
