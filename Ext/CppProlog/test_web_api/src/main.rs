//! Rust Web API
//! Generated using Universal Project Generator

use actix_web::{web, App, HttpResponse, HttpServer, Result};
use serde::{Deserialize, Serialize};

#[derive(Serialize, Deserialize)]
struct HealthCheck {
    status: String,
    message: String,
}

#[derive(Serialize, Deserialize)]
struct HelloResponse {
    message: String,
    generated_by: String,
}

async fn health() -> Result<HttpResponse> {
    Ok(HttpResponse::Ok().json(HealthCheck {
        status: "healthy".to_string(),
        message: "Generated Rust Web API is running".to_string(),
    }))
}

async fn hello(name: web::Path<String>) -> Result<HttpResponse> {
    Ok(HttpResponse::Ok().json(HelloResponse {
        message: format!("Hello, {}! Welcome to the generated Rust Web API! 🎉", name),
        generated_by: "Universal Project Generator".to_string(),
    }))
}

async fn root() -> Result<HttpResponse> {
    Ok(HttpResponse::Ok().json(HelloResponse {
        message: "Welcome to the generated Rust Web API!".to_string(),
        generated_by: "Universal Project Generator with CppProlog".to_string(),
    }))
}

#[actix_web::main]
async fn main() -> std::io::Result<()> {
    println!("🚀 Starting Rust Web API server...");
    println!("Generated using CppProlog and Universal Project Generator");
    
    HttpServer::new(|| {
        App::new()
            .route("/", web::get().to(root))
            .route("/health", web::get().to(health))
            .route("/hello/{name}", web::get().to(hello))
    })
    .bind("127.0.0.1:8080")?
    .run()
    .await
}

#[cfg(test)]
mod tests {
    use super::*;
    use actix_web::{test, web, App};

    #[actix_web::test]
    async fn test_health_endpoint() {
        let app = test::init_service(
            App::new().route("/health", web::get().to(health))
        ).await;
        let req = test::TestRequest::get().uri("/health").to_request();
        let resp = test::call_service(&app, req).await;
        assert!(resp.status().is_success());
    }
}
