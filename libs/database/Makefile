
# Falcon Database Test Makefile

.PHONY: docker-up docker-down docker-clean docker-logs

DOCKER_COMPOSE = docker-compose -f docker-compose.test.yml

docker-up:
	@echo "Starting PostgreSQL test database..."
	@$(DOCKER_COMPOSE) up -d
	@echo "Waiting for PostgreSQL to be ready..."
	@for i in 1 2 3 4 5; do \
		if $(DOCKER_COMPOSE) exec -T postgres pg_isready -U falcon_test >/dev/null 2>&1; then \
			echo "✓ PostgreSQL is ready"; \
			exit 0; \
		fi; \
		echo "  Waiting... ($$i/5)"; \
		sleep 2; \
	done; \
	echo "✗ PostgreSQL failed to start"; \
	exit 1

docker-down:
	@echo "Stopping PostgreSQL test database..."
	@$(DOCKER_COMPOSE) down
	@echo "✓ PostgreSQL stopped"

docker-clean:
	@echo "Cleaning PostgreSQL test data..."
	@$(DOCKER_COMPOSE) down -v
	@echo "✓ PostgreSQL data cleaned"

docker-logs:
	@$(DOCKER_COMPOSE) logs -f postgres
